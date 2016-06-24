/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dd_synthesis_p.hpp"

#include <iostream>
#include <map>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <puma.h>

#include <cudd.h>
#include <cuddInt.h>

#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/clear_circuit.hpp>
#include <core/io/read_pla_to_bdd.hpp>

#define foreach BOOST_FOREACH

namespace revkit
{

namespace internal
{

  using namespace boost::assign;

  typedef boost::graph_traits<dd>::vertex_descriptor dd_node;
  typedef boost::graph_traits<dd>::edge_descriptor   dd_edge;

  struct vertex_drawer
  {
    explicit vertex_drawer( const dd& graph ) : _graph ( graph ) {}

    template<typename Vertex>
    void operator()( std::ostream& os, const Vertex& v ) const
    {
      if ( !boost::in_degree( v, _graph ) || !boost::out_degree( v, _graph ) )
      {
        std::string label;
        if ( out_degree( v, _graph ) )
        {
          label = get_property( _graph, boost::graph_name ).labels.at( get( boost::vertex_name, _graph )[v].var );
        }
        else
        {
          label = boost::lexical_cast<std::string>( get( boost::vertex_name, _graph )[v].var );
        }
        os << "[shape=\"rectangle\",label=\"" << label << "\"]";
      }
      else
      {
        os << "[label=\""
           << get_property( _graph, boost::graph_name ).labels.at( get( boost::vertex_name, _graph )[v].var )
           << ":"
           << get( boost::vertex_name, _graph )[v].dtl
           << "\"]";
      }
    }

  private:
    const dd& _graph;
  };

  struct edge_drawer
  {
  public:
    explicit edge_drawer( const dd& graph ) : _graph( graph ) {}

    template<typename Edge>
    void operator()( std::ostream& os, const Edge& e ) const
    {
      dd_node source = boost::source( e, _graph );

      std::vector<std::string> properties;
      if ( e == *boost::out_edges( source, _graph ).first && in_degree( source, _graph ) )
      {
        properties += "style=dashed";
      }
      if ( get( boost::edge_name, _graph )[e].complemented )
      {
        properties += "color=red";
      }

      if ( properties.size() )
      {
        properties.front().insert( 0, "[" );
        properties.back().append( "]" );
      }

      os << boost::algorithm::join( properties, "," );
    }

  private:
    const dd& _graph;
  };

  void dd_to_dot( const dd& graph, const std::string& filename )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );
    std::ostream os( &fb );
    boost::write_graphviz( os, graph, vertex_drawer( graph ), edge_drawer( graph ) );
    fb.close();
  }

  unsigned dd_node_var( const dd_node& node, const dd& graph )
  {
    return get( boost::vertex_name, graph )[node].var;
  }

  bool dd_node_is_constant( const dd_node& node, const dd& graph )
  {
    return !out_degree( node, graph );
  }

  dd_node dd_root_get_function_root( const dd_node& node, const dd& graph )
  {
    boost::graph_traits<dd>::in_edge_iterator itEdge, itEnd;

    for ( boost::tie( itEdge, itEnd ) = in_edges( node, graph ); itEdge != itEnd; ++itEdge )
    {
      if ( in_degree( source( *itEdge, graph ), graph ) == 0 )
      {
        return source( *itEdge, graph );
      }
    }

    assert( false );
  }

  bool dd_root_is_complemented( const dd_node& node, const dd& graph )
  {
    return get( boost::edge_name, graph )[*out_edges( dd_root_get_function_root( node, graph ), graph ).first].complemented;
  }

  const std::vector<std::string>& dd_labels( const dd& graph )
  {
    return get_property( graph, boost::graph_name ).labels;
  }

  const std::vector<dd_node>& dd_roots( const dd& graph )
  {
    return get_property( graph, boost::graph_name ).roots;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // KFDD Code                                                                  //
  ////////////////////////////////////////////////////////////////////////////////
  dd_from_kfdd_settings::dd_from_kfdd_settings()
    : default_decomposition( 0 ),
      reordering( 0 ),
      sift_factor( 2.5 ),
      sifting_growth_limit( 'a' ),
      sifting_method( 'v' ),
      node_count( 0 )
  {
  }

  bool OKFDD_IsComplement( utnode* node )
  {
    return ((ulint)node & 1) == 1;
  }

  bool OKFDD_IsConstant( dd_man* manager, utnode* node )
  {
    return ( node == manager->OKFDD_ONE || node == manager->OKFDD_ZERO );
  }

  utnode* OKFDD_Regular( utnode* node )
  {
    return (m_and(node));
  }

  bool OKFDD_Value( dd_man* manager, utnode* node )
  {
    if ( OKFDD_IsConstant( manager, node ) )
    {
      return node == manager->OKFDD_ONE;
    }
    else
    {
      assert( false );
    }
  }

  dd_node dd_from_kfdd( dd& graph, dd_man* manager, utnode* node, std::map<utnode*, dd_node>& visited_map )
  {
    if ( visited_map.find( node ) != visited_map.end() )
    {
      return visited_map[node];
    }

    dd_node v = boost::add_vertex( graph );

    if ( !OKFDD_IsConstant( manager, node ) )
    {
      dd_node low  = dd_from_kfdd( graph, manager, OKFDD_Regular( (m_and(node))->lo_p ), visited_map );
      dd_node high = dd_from_kfdd( graph, manager, OKFDD_Regular( (m_and(node))->hi_p ), visited_map );

      add_edge( v, low, graph );
      dd_edge edge;
      bool    ok;
      boost::tie( edge, ok ) = add_edge( v, high, graph );
      get( boost::edge_name, graph )[edge].complemented = OKFDD_IsComplement( (m_and(node))->hi_p );

      get( boost::vertex_name, graph )[v].var = manager->OKFDD_Label( node ) - 1;
      get( boost::vertex_name, graph )[v].dtl = (unsigned short)manager->OKFDD_PI_DTL_Table[manager->OKFDD_Label( node )];
    }
    else
    {
      get( boost::vertex_name, graph )[v].var = OKFDD_Value( manager, node );
    }

    visited_map[node] = v;

    return v;
  }

  void dd_from_kfdd( dd& graph, dd_man* manager, const std::vector<utnode*>& nodes )
  {
    std::map<utnode*, dd_node> visited_map;

    for ( std::vector<utnode*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
      dd_node v = boost::add_vertex( graph );
      bool ok;
      dd_edge edge;
      dd_node v2 = dd_from_kfdd( graph, manager, *it, visited_map );
      boost::tie( edge, ok ) = add_edge( v, v2, graph );
      get( boost::edge_name, graph )[edge].complemented = OKFDD_IsComplement( *it );

      get( boost::vertex_name, graph )[v].var      = get_property( graph, boost::graph_name ).labels.size() - nodes.size() + ( it - nodes.begin() );
      get_property( graph, boost::graph_name ).roots += v2;
    }
  }

  void dd_from_kfdd( dd& graph, const std::string& filename, const dd_from_kfdd_settings& settings )
  {
    unsigned char      ut_hashsize  = 0;
    unsigned long      ct_hashsize  = 5003;
    unsigned long int  rc_cachesize = 1000;
    unsigned char      ct_searchlen = 3;
    unsigned short int var_lim     = 20000;

    dd_man* dd_manager = OKFDD_Init( ut_hashsize,
                                     ct_hashsize,
                                     rc_cachesize,
                                     ct_searchlen,
                                     var_lim );

    /* set some construction parameters */
    dd_manager->OKFDD_Outputflags  = 0;
    dd_manager->OKFDD_Version_Wait = false;
    dd_manager->OKFDD_DTL_Default  = settings.default_decomposition;
    dd_manager->OKFDD_Tempfactor   = 3;
    dd_manager->OKFDD_Siftfactor   = 3;
    dd_manager->OKFDD_Temproutine  = 3;
    dd_manager->OKFDD_Interleaving = true;

    std::string input_names, output_names;
    dd_manager->OKFDD_Read_BLIF( const_cast<char*>( filename.c_str() ), 0, &INTERHAND, 0, 0, &input_names, &output_names );

    unsigned npi = dd_manager->OKFDD_P_I;
    unsigned npo = dd_manager->OKFDD_P_O;

    switch ( settings.reordering )
    {
    case 0: //kfdd_synthesis_reordering_none:
      break;

    case 1: //kfdd_synthesis_reordering_exact_dtl_friedman:
      dd_manager->OKFDD_DTL_Friedman( 0, npi - 1 );
      break;

    case 2: //kfdd_synthesis_reordering_exact_dtl_permutation:
      dd_manager->OKFDD_DTL_Permutation( 0, npi - 1 );
      break;

    case 3: //kfdd_synthesis_reordering_dtl_sifting:
      dd_manager->OKFDD_DTL_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      break;

    case 4: //kfdd_synthesis_reordering_exact_friedman:
      dd_manager->OKFDD_Friedman( 0, npi - 1 );
      break;

    case 5: //kfdd_synthesis_reordering_exact_permutation:
      dd_manager->OKFDD_Permutation( 0, npi - 1 );
      break;

    case 6: //kfdd_synthesis_reordering_sifting:
      dd_manager->OKFDD_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      break;

    case 7: //kfdd_synthesis_reordering_sifting_and_dtl_sifting:
      dd_manager->OKFDD_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      dd_manager->OKFDD_DTL_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      break;

    case 8: //kfdd_synthesis_reordering_inverse:
      dd_manager->OKFDD_Inversion( 0, npi - 1 );
      break;

    case 9: //kfdd_synthesis_reordering_quantum_sifting line-sifting:
      dd_manager->OKFDD_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      dd_manager->OKFDD_DTL_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      dd_manager->OKFDD_DTL_Sifting_Quantum( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method, 1 );
      break;

    case 10: //kfdd_synthesis_reordering_quantum_sifting quantumcost-sifting:
      dd_manager->OKFDD_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      dd_manager->OKFDD_DTL_Sifting( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method );
      dd_manager->OKFDD_DTL_Sifting_Quantum( 0, npi - 1, settings.sift_factor, settings.sifting_growth_limit, settings.sifting_method, 0 );
      break;
    }

    std::vector<std::string> labels;
    boost::algorithm::trim( input_names );
    boost::algorithm::trim( output_names );
    boost::algorithm::split( labels, input_names, boost::is_any_of( " " ) );
    get_property( graph, boost::graph_name ).ninputs = labels.size();
    std::copy( labels.begin(), labels.end(), std::back_inserter( get_property( graph, boost::graph_name ).labels ) );
    boost::algorithm::split( labels, output_names, boost::is_any_of( " " ) );
    std::copy( labels.begin(), labels.end(), std::back_inserter( get_property( graph, boost::graph_name ).labels ) );

    std::vector<utnode*> nodes;
    for ( unsigned i = 0; i < npo; ++i )
    {
      nodes += dd_manager->OKFDD_PO_Root_Table( dd_manager->OKFDD_PO_Table( i ) );
    }

    dd_from_kfdd( graph, dd_manager, nodes );

    if ( settings.node_count )
    {
      // TODO node counting seems to be wrong in PUMA
      *settings.node_count = dd_manager->OKFDD_Size_all();
      //*settings.node_count = dd_manager->OKFDD_Now_size_i;
      // TODO This fixes node counting
      //*settings.node_count = boost::num_vertices( graph ) - 1u - ( get_property( graph, boost::graph_name ).labels.size() - get_property( graph, boost::graph_name ).ninputs );
    }
  }

////////////////////////////////////////////////////////////////////////////////
// BDD Code                                                                   //
////////////////////////////////////////////////////////////////////////////////

  dd_from_bdd_settings::dd_from_bdd_settings()
    : complemented_edges( true ),
      reordering( CUDD_REORDER_SIFT ),
      node_count( 0 )
  {
  }

  dd_node dd_from_bdd( dd& graph, DdNode* node, std::map<DdNode*, dd_node>& visited_map )
  {
    if ( visited_map.find( node ) != visited_map.end() )
    {
      return visited_map[node];
    }

    dd_node v = boost::add_vertex( graph );

    if ( !cuddIsConstant( node ) )
    {
      dd_node low  = dd_from_bdd( graph, Cudd_Regular( cuddE( node ) ), visited_map );
      dd_node high = dd_from_bdd( graph, cuddT( node ), visited_map );

      dd_edge edge;
      bool    ok;
      boost::tie( edge, ok ) = add_edge( v, low, graph );
      get( boost::edge_name, graph )[edge].complemented = Cudd_IsComplement( cuddE( node ) );

      add_edge( v, high, graph );

      get( boost::vertex_name, graph )[v].var = node->index;
    }
    else
    {
      get( boost::vertex_name, graph )[v].var = cuddV( node );
    }

    visited_map[node] = v;

    return v;
  }

  void dd_from_bdd( dd& graph, const std::vector<DdNode*>& nodes )
  {
    std::map<DdNode*, dd_node> visited_map;

    for ( std::vector<DdNode*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
      dd_node v = boost::add_vertex( graph );
      bool ok;
      dd_edge edge;
      dd_node v2 = dd_from_bdd( graph, Cudd_Regular( *it ), visited_map );
      boost::tie( edge, ok ) = add_edge( v, v2, graph );
      get( boost::edge_name, graph )[edge].complemented = Cudd_IsComplement( *it );

      get( boost::vertex_name, graph )[v].var = get_property( graph, boost::graph_name ).labels.size() - nodes.size() + ( it - nodes.begin() );
      get_property( graph, boost::graph_name ).roots += v2;
    }
  }

  void dd_from_bdd( dd& graph, const std::string& filename, const dd_from_bdd_settings& settings )
  {
    BDDTable bdd;
    read_pla_to_bdd( bdd, filename );

    // Reorder the BDD
    Cudd_ReduceHeap( bdd.cudd, (Cudd_ReorderingType)settings.reordering, 0 );

    // Node Count
    if ( settings.node_count )
    {
      *settings.node_count = Cudd_ReadNodeCount( bdd.cudd );
    }

    // statistics about the BDD
    if ( settings.infofilename.size() )
    {
      FILE *fp = 0;
      fp = fopen( settings.infofilename.c_str(), "w" );
      Cudd_PrintInfo( bdd.cudd, fp );
      fclose( fp );
    }

    // complemented edges
    if ( !settings.complemented_edges )
    {
      for ( unsigned i = 0; i < bdd.outputs.size(); ++i )
      {
        DdNode *tmp = Cudd_BddToAdd( bdd.cudd, bdd.outputs[i].second );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, bdd.outputs[i].second );
        bdd.outputs[i].second = tmp;
      }
    }

    using boost::adaptors::map_keys;
    using boost::adaptors::map_values;

    std::vector<DdNode*> nodes;
    boost::push_back( nodes, bdd.outputs | map_values );

    boost::push_back( get_property( graph, boost::graph_name ).labels, bdd.inputs | map_keys );
    boost::push_back( get_property( graph, boost::graph_name ).labels, bdd.outputs | map_keys );

    get_property( graph, boost::graph_name ).ninputs = bdd.inputs.size();

    dd_from_bdd( graph, nodes );
  }

  ////////////////////////////////////////////////////////////////////////////////
  // DD Synthesis                                                               //
  ////////////////////////////////////////////////////////////////////////////////
  struct data
  {
    unsigned lines;

    std::vector<int> constantValue;
    std::vector<int> lineNeeded;
    std::map<dd_node, unsigned> node2line;

    unsigned up( unsigned cv )
    {
      unsigned ret = lines++;
      lineNeeded += -1;
      constantValue += cv;
      return ret;
    }
  };

  int reversible_generator( circuit& circ, data& d, unsigned index, unsigned dtl, int low, int high, bool low_complemented, bool high_complemented )
  {
    if ( low >= 0 && high >= 0 )
    {
      if ( low == high ) // ll case
      {
        switch ( dtl )
        {
        case 0:
          if ( high_complemented )
          {
            if ( d.lineNeeded[high] )
            {
              unsigned tmpLine = d.up( 0 );

              append_cnot( circ, index, tmpLine );
              append_cnot( circ, low, tmpLine );

              return tmpLine;
            }
            else
            {
              append_cnot( circ, index, low );

              return low;
            }
          }
          else if ( low_complemented )
          {
            if ( d.lineNeeded[high] )
            {
              unsigned tmpLine = d.up( 1 );

              append_cnot( circ, index, tmpLine );
              append_cnot( circ, low, tmpLine );

              return tmpLine;
            }
            else
            {
              append_cnot( circ, index, low );
              append_not( circ, low );

              return low;
            }
          }
          break;

        case 1:
          if ( high_complemented )
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, low )( tmpLine );
            append_cnot( circ, index, tmpLine );
            append_cnot( circ, low, tmpLine );

            return tmpLine;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, low )( tmpLine );
            append_cnot( circ, low, tmpLine );

            return tmpLine;
          }
          break;

        case 2:
          if ( high_complemented )
          {
            unsigned tmpLine = d.up( 1 );

            append_toffoli( circ )( index, low )( tmpLine );
            append_cnot( circ, index, tmpLine );

            return tmpLine;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, low )( tmpLine );

            return tmpLine;
          }
          break;
        }
      }
      else if ( d.lineNeeded[low] || d.lineNeeded[high] )
      {
        switch ( dtl )
        {
        case 0:
          if ( high_complemented )
          {
            unsigned tmpLine = d.up( 0 );

            append_cnot( circ, index, tmpLine );
            append_cnot( circ, low, tmpLine );
            append_toffoli( circ )( index, high )( tmpLine );
            append_toffoli( circ )( index, low )( tmpLine );

            return tmpLine;
          }
          else if ( low_complemented )
          {
            unsigned tmpLine = d.up( 1 );

            append_cnot( circ, index, tmpLine );
            append_cnot( circ, low, tmpLine );
            append_toffoli( circ )( index, high )( tmpLine );
            append_toffoli( circ )( index, low )( tmpLine );

            return tmpLine;
          }
          else
          {
            unsigned tmpLine = d.up( 0 );

            append_cnot( circ, low, tmpLine );
            append_toffoli( circ )( index, high )( tmpLine );
            append_toffoli( circ )( index, low )( tmpLine );

            return tmpLine;
          }
          break;

        case 1:
          if ( high_complemented )
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, high )( tmpLine );
            append_cnot( circ, low, tmpLine );
            append_cnot( circ, index, tmpLine );

            return tmpLine;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, high )( tmpLine );
            append_cnot( circ, low, tmpLine );

            return tmpLine;
          }
          break;

        case 2:
          if ( high_complemented )
          {
            unsigned tmpLine = d.up( 1 );

            append_toffoli( circ )( index, high )( tmpLine );
            append_cnot( circ, low, tmpLine );
            append_cnot( circ, high, tmpLine );
            append_cnot( circ, index, tmpLine );

            return tmpLine;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            unsigned tmpLine = d.up( 0 );

            append_toffoli( circ )( index, high )( tmpLine );
            append_cnot( circ, low, tmpLine );
            append_cnot( circ, high, tmpLine );

            return tmpLine;
          }
          break;
        }
      }
      else
      {
        switch ( dtl )
        {
        case 0:
          if ( high_complemented )
          {
            append_toffoli( circ )( index, low )( high );
            append_cnot( circ, index, low );
            append_toffoli( circ )( high, index )( low );

            return low;
          }
          else if ( low_complemented )
          {
            append_not( circ, low );
            append_toffoli( circ )( low, index )( high );
            append_toffoli( circ )( high, index )( low );

            return low;
          }
          else
          {
            append_cnot( circ, low, high );
            append_toffoli( circ )( high, index )( low );

            return low;
          }
          break;

        case 1:
          if ( high_complemented )
          {
            append_toffoli( circ )( index, high )( low );
            append_cnot( circ, index, low );

            return low;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            append_toffoli( circ )( index, high )( low );

            return low;
          }
          break;

        case 2:
          if ( high_complemented )
          {
            append_not( circ, high );
            append_toffoli( circ )( index, high )( low );
            append_cnot( circ, low, high );

            return high;
          }
          else if ( low_complemented )
          {
          }
          else
          {
            append_toffoli( circ )( index, high )( low );
            append_cnot( circ, low, high );

            return high;
          }
          break;
        }
      }
    }
    else if ( low == -1 && high >= 0 )
    {
      switch ( dtl )
      {
      case 0:
        if ( high_complemented )
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( index, high )( tmpLine );

          return tmpLine;
        }
        else if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( index, high )( tmpLine );
          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        break;

      case 1:
        if ( high_complemented )
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( index, high )( tmpLine );
          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        else if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( index, high )( tmpLine );

          return tmpLine;
        }
        break;

      case 2:
        if ( high_complemented )
        {
          unsigned tmpLine = d.up( 0 );

          append_toffoli( circ )( index, high )( tmpLine );
          append_cnot( circ, high, tmpLine );
          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        else if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( index, high )( tmpLine );
          append_cnot( circ, high, tmpLine );

          return tmpLine;
        }
      }
    }
    else if ( low == -2 && high >= 0 )
    {
      switch ( dtl )
      {
      case 0:
        if ( high_complemented )
        {
        }
        else if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 0 );

          append_toffoli( circ )( index, high )( tmpLine );

          return tmpLine;
        }
        break;
      }
    }
    else if ( low >= 0 && high == -1 )
    {
      switch ( dtl )
      {
      case 0:
        if ( high_complemented )
        {
        }
        else if ( low_complemented )
        {
          unsigned tmpLine = d.up( 1 );

          append_toffoli( circ )( low, index )( tmpLine );
          append_cnot( circ, low, tmpLine );

          return tmpLine;
        }
        else
        {
          unsigned tmpLine = d.up( 0 );

          append_cnot( circ, index, tmpLine );
          append_toffoli( circ )( low, index )( tmpLine );
          append_cnot( circ, low, tmpLine );

          return tmpLine;
        }
        break;

      case 1:
        if ( high_complemented )
        {
        }
        else if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 0 );

          append_cnot( circ, low, tmpLine );
          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        break;

      case 2:
        if ( high_complemented )
        {
        }
        if ( low_complemented )
        {
        }
        else
        {
          unsigned tmpLine = d.up( 1 );

          append_cnot( circ, low, tmpLine );
          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        break;
      }
    }
    else if ( low >= 0 && high == -2 )
    {
      switch ( dtl )
      {
      case 0:
        if ( high_complemented )
        {
        }
        else if ( low_complemented )
        {
          unsigned tmpLine = d.up( 1 );

          append_cnot( circ, index, tmpLine );
          append_toffoli( circ )( low, index )( tmpLine );
          append_cnot( circ, low, tmpLine );

          return tmpLine;
        }
        else
        {
          unsigned tmpLine = d.up( 0 );

          append_toffoli( circ )( low, index )( tmpLine );
          append_cnot( circ, low, tmpLine );

          return tmpLine;
        }
        break;
      }
    }
    else if ( low == -1 && high == -1 )
    {
      assert( dtl != 0 );
      assert( !low_complemented && !high_complemented );

      switch ( dtl )
      {
      case 1:
        {
          unsigned tmpLine = d.up( 1 );

          append_cnot( circ, index, tmpLine );

          return tmpLine;
        }
        break;

      case 2:
        return index;
        break;
      }
    }
    else if ( low == -1 && high == -2 )
    {
      assert( dtl == 0 );
      unsigned tmpLine = d.up( 1 );

      append_cnot( circ, index, tmpLine );

      return tmpLine;
    }
    else if ( low == -2 && high == -1 )
    {
      return index;
    }

    std::cerr << "Missing: index: " << index << " dtl: " << dtl << " low: " << low << " high: " << high << " low_complemented: " << low_complemented << " high_complemented: " << high_complemented  << std::endl;

    assert( false );

    return -1;
  }

  int node2line( const dd_node& node, const dd& graph, bool is_complemented, data& d )
  {
    int line = -1;

    if ( dd_node_is_constant( node, graph ) )
    {
      if ( is_complemented )
      {
        line = dd_node_var( node, graph ) == 1 ? -2 : -1;
      }
      else
      {
        line = dd_node_var( node, graph ) == 1 ? -1 : -2;
      }
    }
    else
    {
      std::map<dd_node, unsigned>::const_iterator it = d.node2line.find( node );
      assert( it != d.node2line.end() );
      assert( d.lineNeeded[it->second] > 0 || it->second < get_property( graph, boost::graph_name ).ninputs );
      line = it->second;
      if ( line >= (int)get_property( graph, boost::graph_name ).ninputs )
      {
        --d.lineNeeded[line];
      }
    }

    return line;
  }

  void dd_synthesis( circuit& circ, data& d, const dd_node& node, const dd& graph, boost::function<int(circuit&, data&, unsigned, unsigned short, int, int, bool, bool)> gate_inserter )
  {

    if ( dd_node_is_constant( node, graph ) )
    {
      // should only happen, if PO is constant
      unsigned tmpLine = d.lines;
      ++d.lines;
      d.lineNeeded += -1;
      d.constantValue += (int)dd_node_var( node, graph );
      d.node2line.insert( std::make_pair( node, tmpLine ) );
      return;
    }

    if ( d.node2line.find( node ) != d.node2line.end() )
    {
      // already visited
      return;
    }

    // get low and high
    bool low_complemented, high_complemented;
    dd_node lowNode, highNode;

    boost::graph_traits<dd>::out_edge_iterator itEdge = out_edges( node, graph ).first;
    low_complemented = get( boost::edge_name, graph )[*itEdge].complemented;
    lowNode = target( *itEdge, graph );
    ++itEdge;
    high_complemented = get( boost::edge_name, graph )[*itEdge].complemented;
    highNode = target( *itEdge, graph );

    if ( !dd_node_is_constant( highNode, graph ) )
    {
      dd_synthesis( circ, d, highNode, graph, gate_inserter );
    }
    if ( !dd_node_is_constant( lowNode, graph ) )
    {
      dd_synthesis( circ, d, lowNode, graph, gate_inserter );
    }

    unsigned index = dd_node_var( node, graph );
    int high       = node2line( highNode, graph, high_complemented, d );
    int low        = node2line( lowNode, graph, low_complemented, d );

    if ( high < 0 )
    {
      high_complemented = false;
    }
    if ( low < 0 )
    {
      low_complemented = false;
    }

    unsigned short dtl = get( boost::vertex_name, graph )[node].dtl;
    int out            = gate_inserter( circ, d, index, dtl, low, high, low_complemented, high_complemented );

    assert( out != -1 );
    d.node2line.insert( std::make_pair( node, out ) );
    assert( (int)d.lineNeeded.size() > out );

    d.lineNeeded[out] = in_degree( node, graph );

  }

  void dd_synthesis( circuit& circ, const dd& graph, boost::function<int(circuit&, data&, unsigned, unsigned short, int, int, bool, bool)> gate_inserter )
  {
    // empty circuit
    clear_circuit( circ );

    // get number of inputs
    unsigned ninputs = get_property( graph, boost::graph_name ).ninputs;

    data d;
    d.lines = ninputs;
    d.constantValue.resize( d.lines, -1 );
    d.lineNeeded.resize( d.lines, -1 );

    // recursion
    foreach ( const unsigned& node_index, dd_roots( graph ) )
    {
      dd_synthesis( circ, d, node_index, graph, gate_inserter );
    }

    foreach ( const unsigned& node_index, dd_roots( graph ) )
    {
      if ( dd_root_is_complemented( node_index, graph ) )
      {
        append_not( circ, d.node2line[node_index] );
      }
    }

    circ.set_lines( d.lines );

    // set inputs and constants
    std::vector<std::string> inputs( d.lines );
    std::vector<constant> constants( d.lines, constant() );
    std::copy( dd_labels( graph ).begin(), dd_labels( graph ).begin() + ninputs, inputs.begin() );
    std::transform( d.constantValue.begin() + ninputs, d.constantValue.end(), inputs.begin() + ninputs, boost::lexical_cast<std::string, int> );
    std::transform( d.constantValue.begin() + ninputs, d.constantValue.end(), constants.begin() + ninputs, boost::lexical_cast<bool, int> );
    circ.set_inputs( inputs );
    circ.set_constants( constants );

    // set outputs and garbage
    std::vector<std::string> outputs( d.lines, "g" );
    std::vector<bool> garbage( d.lines, true );
    foreach ( const unsigned& node_index, dd_roots( graph ) )
    {
      unsigned index = d.node2line[node_index];
      outputs[index] = dd_labels( graph ).at( get( boost::vertex_name, graph )[dd_root_get_function_root( node_index, graph )].var );
      garbage[index] = false;
    }
    circ.set_outputs( outputs );
    circ.set_garbage( garbage );

  }

  void dd_synthesis( circuit& circ, const dd& graph )
  {
    dd_synthesis( circ, graph, &reversible_generator );
  }


}

}
