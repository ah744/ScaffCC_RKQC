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

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include <core/circuit.hpp>
#include <core/gate.hpp>
#include <core/pattern.hpp>

#include <core/functions/add_circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/add_line_to_circuit.hpp>
#include <core/functions/circuit_hierarchy.hpp>
#include <core/functions/circuit_to_truth_table.hpp>
#include <core/functions/clear_circuit.hpp>
#include <core/functions/copy_circuit.hpp>
#include <core/functions/copy_metadata.hpp>
#include <core/functions/control_lines.hpp>
#include <core/functions/create_simulation_pattern.hpp>
#include <core/functions/expand_circuit.hpp>
#include <core/functions/extend_truth_table.hpp>
#include <core/functions/flatten_circuit.hpp>
#include <core/functions/find_lines.hpp>
#include <core/functions/fully_specified.hpp>
#include <core/functions/reverse_circuit.hpp>
#include <core/functions/target_lines.hpp>

#include <algorithms/simulation/simple_simulation.hpp>

using namespace boost::python;
using namespace revkit;

void py_append_circuit( circuit& circ, const circuit& src, object controls )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  append_circuit( circ, src, c );
}

void py_prepend_circuit( circuit& circ, const circuit& src, object controls )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  prepend_circuit( circ, src, c );
}

void py_insert_circuit( circuit& circ, unsigned pos, const circuit& src, object controls )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  insert_circuit( circ, pos, src, c );
}

gate& append_toffoli1( circuit& circ, object controls, unsigned target )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return append_toffoli( circ, c, target );
}

gate& prepend_toffoli1( circuit& circ, object controls, unsigned target )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return prepend_toffoli( circ, c, target );
}

gate& insert_toffoli1( circuit& circ, unsigned pos, object controls, unsigned target )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return insert_toffoli( circ, pos, c, target );
}

gate& append_fredkin1( circuit& circ, object controls, unsigned target1, unsigned target2 )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return append_fredkin( circ, c, target1, target2 );
}

gate& prepend_fredkin1( circuit& circ, object controls, unsigned target1, unsigned target2 )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return prepend_fredkin( circ, c, target1, target2 );
}

gate& insert_fredkin1( circuit& circ, unsigned pos, object controls, unsigned target1, unsigned target2 )
{
  gate::line_container c;
  stl_input_iterator<unsigned> begin( controls ), end;
  std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );

  return insert_fredkin( circ, pos, c, target1, target2 );
}

gate& append_module1( circuit& circ, const std::string& name, object controls, object targets )
{
  gate::line_container c;
  {
    stl_input_iterator<unsigned> begin( controls ), end;
    std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  }
  std::vector<unsigned> t;
  {
    stl_input_iterator<unsigned> begin( targets ), end;
    std::copy( begin, end, std::back_inserter( t ) );
  }

  return append_module( circ, name, c, t );
}

gate& prepend_module1( circuit& circ, const std::string& name, object controls, object targets )
{
  gate::line_container c;
  {
    stl_input_iterator<unsigned> begin( controls ), end;
    std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  }
  std::vector<unsigned> t;
  {
    stl_input_iterator<unsigned> begin( targets ), end;
    std::copy( begin, end, std::back_inserter( t ) );
  }

  return prepend_module( circ, name, c, t );
}

gate& insert_module1( circuit& circ, unsigned n, const std::string& name, object controls, object targets )
{
  gate::line_container c;
  {
    stl_input_iterator<unsigned> begin( controls ), end;
    std::copy( begin, end, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  }
  std::vector<unsigned> t;
  {
    stl_input_iterator<unsigned> begin( targets ), end;
    std::copy( begin, end, std::back_inserter( t ) );
  }

  return insert_module( circ, n, name, c, t );
}

BOOST_PYTHON_FUNCTION_OVERLOADS( add_line_to_circuit_overloads, add_line_to_circuit, 3, 5 )

std::string hierarchy_tree_node_name( const hierarchy_tree& tree, hierarchy_tree::node_type node )
{
  return boost::get<0>( boost::get( boost::vertex_name, tree.graph() )[node] );
}

const circuit& hierarchy_tree_node_circuit( const hierarchy_tree& tree, hierarchy_tree::node_type node )
{
  return *boost::get<1>( boost::get( boost::vertex_name, tree.graph() )[node] );
}

list hierarchy_tree_node_children( const hierarchy_tree& tree, hierarchy_tree::node_type node )
{
  list l;
  std::vector<hierarchy_tree::node_type> children;
  tree.children( node, children );
  std::for_each( children.begin(), children.end(), boost::bind( &list::append<hierarchy_tree::node_type>, &l, _1 ) );

  return l;
}

unsigned hierarchy_tree_size( const hierarchy_tree& tree )
{
  return num_vertices( tree.graph() );
}

void (*copy_metadata1)(const truth_table<boost::optional<bool> >&, circuit&) = copy_metadata<boost::optional<bool> >;

void copy_metadata2( const circuit& base, circuit& circ, const copy_metadata_settings& settings )
{
  copy_metadata( base, circ, settings );
}

object py_create_simulation_pattern( const pattern& p, const circuit& circ )
{
  std::vector<boost::dynamic_bitset<> > sim;
  std::map<std::string, boost::dynamic_bitset<> > init;
  std::string error;

  if ( create_simulation_pattern( p, circ, sim, init, &error ) )
  {
    list py_sim;

    foreach ( const boost::dynamic_bitset<>& step, sim )
    {
      list py_step;
      foreach ( unsigned idx, boost::irange( (size_t)0, step.size() ) )
      {
        py_step.append( step.test( idx ) );
      }
      py_sim.append( py_step );
    }

    dict py_init;

    typedef std::map<std::string, boost::dynamic_bitset<> >::value_type pair_t;
    foreach ( const pair_t& pair, init )
    {
      list bv;
      foreach ( unsigned idx, boost::irange( (size_t)0, pair.second.size() ) )
      {
        bv.append( pair.second.test( idx ) );
      }
      py_init[pair.first] = bv;
    }

    dict ret;
    ret["pattern"] = py_sim;
    ret["init"] = py_init;

    return ret;
  }
  else
  {
    return object( error );
  }
}

// NOTE to be done for Version 1.1
//bool (*expand_circuit1)(const circuit&, circuit&, unsigned, const std::vector<unsigned>&) = expand_circuit;
bool (*expand_circuit2)(const circuit&, circuit&) = expand_circuit;

/* find_lines */
list find_non_empty_lines1( const gate& g )
{
  gate::line_container c;
  list l;
  find_non_empty_lines( g, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list find_non_empty_lines2( const circuit& circ, unsigned first, unsigned last )
{
  gate::line_container c;
  list l;
  find_non_empty_lines( circ.begin() + first, circ.begin() + last, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list find_non_empty_lines3( const circuit& circ )
{
  gate::line_container c;
  list l;
  find_non_empty_lines( circ, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list find_empty_lines1( const gate& g, unsigned line_size )
{
  gate::line_container c;
  list l;
  find_empty_lines( g, line_size, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list find_empty_lines2( const circuit& circ, unsigned first, unsigned last )
{
  gate::line_container c;
  list l;
  find_empty_lines( circ.begin() + first, circ.begin() + last, circ.lines(), std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list find_empty_lines3( const circuit& circ )
{
  gate::line_container c;
  list l;
  find_empty_lines( circ, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

bool (*fully_specified1)(const binary_truth_table&, bool) = fully_specified;
BOOST_PYTHON_FUNCTION_OVERLOADS( fully_specified1_overloads, fully_specified, 1, 2 )

void (*reverse_circuit1)(const circuit&, circuit&) = reverse_circuit;
void (*reverse_circuit2)(circuit&) = reverse_circuit;

/* control and target lines */
list control_lines1( const gate& g )
{
  gate::line_container c;
  list l;
  control_lines( g, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

list target_lines1( const gate& g )
{
  gate::line_container c;
  list l;
  target_lines( g, std::insert_iterator<gate::line_container>( c, c.begin() ) );
  std::for_each( c.begin(), c.end(), boost::bind( &list::append<unsigned>, &l, _1 ) );
  return l;
}

void export_core_functions()
{

  // core/functions
  def( "py_append_circuit", py_append_circuit );
  def( "py_prepend_circuit", py_prepend_circuit );
  def( "py_insert_circuit", py_insert_circuit );

  def( "py_append_cnot", append_cnot, return_internal_reference<1>() );
  def( "py_append_not", append_not, return_internal_reference<1>() );
  def( "py_append_toffoli", append_toffoli1, return_internal_reference<1>() );
  def( "py_append_fredkin", append_fredkin1, return_internal_reference<1>() );
  def( "py_append_v", append_v, return_internal_reference<1>() );
  def( "py_append_vplus", append_vplus, return_internal_reference<1>() );
  def( "py_append_peres", append_peres, return_internal_reference<1>() );
  def( "py_append_module", append_module1, return_internal_reference<1>() );
  def( "py_prepend_cnot", prepend_cnot, return_internal_reference<1>() );
  def( "py_prepend_not", prepend_not, return_internal_reference<1>() );
  def( "py_prepend_toffoli", prepend_toffoli1, return_internal_reference<1>() );
  def( "py_prepend_fredkin", prepend_fredkin1, return_internal_reference<1>() );
  def( "py_prepend_v", prepend_v, return_internal_reference<1>() );
  def( "py_prepend_vplus", prepend_vplus, return_internal_reference<1>() );
  def( "py_prepend_peres", append_peres, return_internal_reference<1>() );
  def( "py_prepend_module", prepend_module1, return_internal_reference<1>() );
  def( "py_insert_cnot", insert_cnot, return_internal_reference<1>() );
  def( "py_insert_not", insert_not, return_internal_reference<1>() );
  def( "py_insert_toffoli", insert_toffoli1, return_internal_reference<1>() );
  def( "py_insert_fredkin", insert_fredkin1, return_internal_reference<1>() );
  def( "py_insert_v", insert_v, return_internal_reference<1>() );
  def( "py_insert_vplus", insert_vplus, return_internal_reference<1>() );
  def( "py_insert_peres", append_peres, return_internal_reference<1>() );
  def( "py_insert_module", insert_module1, return_internal_reference<1>() );
  // further commands

  def( "py_add_line_to_circuit", add_line_to_circuit, add_line_to_circuit_overloads() );

  class_<hierarchy_tree>( "hierarchy_tree" )
    .def( "root", &hierarchy_tree::root )
    .def( "node_name", hierarchy_tree_node_name )
    .def( "node_circuit", hierarchy_tree_node_circuit, return_internal_reference<1>() )
    .def( "children", hierarchy_tree_node_children )
    .def( "parent", &hierarchy_tree::parent )
    .def( "size", hierarchy_tree_size )
    ;
  def( "circuit_hierarchy", circuit_hierarchy );

  def( "py_circuit_to_truth_table", circuit_to_truth_table );
  def( "py_clear_circuit", clear_circuit );
  def( "py_copy_circuit", copy_circuit );
  class_<copy_metadata_settings>( "copy_metadata_settings" )
    .def_readwrite( "copy_inputs", &copy_metadata_settings::copy_inputs )
    .def_readwrite( "copy_outputs", &copy_metadata_settings::copy_outputs )
    .def_readwrite( "copy_constants", &copy_metadata_settings::copy_constants )
    .def_readwrite( "copy_garbage", &copy_metadata_settings::copy_garbage )
    .def_readwrite( "copy_name", &copy_metadata_settings::copy_name )
    .def_readwrite( "copy_inputbuses", &copy_metadata_settings::copy_inputbuses )
    .def_readwrite( "copy_outputbuses", &copy_metadata_settings::copy_outputbuses )
    .def_readwrite( "copy_statesignals", &copy_metadata_settings::copy_statesignals )
    .def_readwrite( "copy_modules", &copy_metadata_settings::copy_modules )
    ;
  def( "py_copy_metadata", copy_metadata1 );
  def( "py_copy_metadata", copy_metadata2 );
  def( "py_control_lines", control_lines1 );
  def( "py_create_simulation_pattern", py_create_simulation_pattern );
  //def( "py_expand_circuit", expand_circuit1 );
  def( "py_expand_circuit", expand_circuit2 );
  def( "py_extend_truth_table", extend_truth_table );

  def( "py_find_non_empty_lines", find_non_empty_lines1 );
  def( "py_find_non_empty_lines", find_non_empty_lines2 );
  def( "py_find_non_empty_lines", find_non_empty_lines3 );
  def( "py_find_empty_lines", find_empty_lines1 );
  def( "py_find_empty_lines", find_empty_lines2 );
  def( "py_find_empty_lines", find_empty_lines3 );

  def( "py_flatten_circuit", flatten_circuit );

  def( "py_fully_specified", fully_specified1, fully_specified1_overloads() );
  def( "py_reverse_circuit", reverse_circuit1 );
  def( "py_reverse_circuit", reverse_circuit2 );
  def( "py_target_lines", target_lines1 );

}
