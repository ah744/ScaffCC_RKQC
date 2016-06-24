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

#include "quantum_decomposition.hpp"

#include <boost/assign/std/vector.hpp>

#include <core/target_tags.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/add_line_to_circuit.hpp>
#include <core/functions/clear_circuit.hpp>
#include <core/functions/copy_metadata.hpp>
#include <core/functions/find_lines.hpp>
#include <core/utils/timer.hpp>

using namespace boost::assign;

namespace revkit
{

  struct inverse_peres_tag {};

  bool is_inverse_peres( const gate& g )
  {
    return is_type<inverse_peres_tag>( g.type() );
  }

  void standard_decomposition::operator()( circuit& circ, const gate& g ) const
  {
    if ( is_v( g ) || is_vplus( g ) )
    {
      circ.append_gate() = g;
    }
    else if ( is_toffoli( g ) )
    {
      unsigned num_controls = std::distance( g.begin_controls(), g.end_controls() );

      if ( num_controls <= 1 )
      {
        circ.append_gate() = g;
      }
      else if ( num_controls == 2 )
      {
        gate::const_iterator it = g.begin_controls();
        unsigned c1 = *it++;
        unsigned c2 = *it;
        unsigned t  = *g.begin_targets();

        append_v( circ, c2, t );
        append_cnot( circ, c1, c2 );
        append_vplus( circ, c2, t );
        append_cnot( circ, c1, c2 );
        append_v( circ, c1, t );
      }
      else
      {
        // get the empty lines
        std::vector<unsigned> empty_lines;
        find_empty_lines( g, circ.lines(), std::back_inserter( empty_lines ) );

        // get the control lines
        std::vector<unsigned> control_lines( num_controls );
        std::copy( g.begin_controls(), g.end_controls(), control_lines.begin() );
        std::sort( control_lines.begin(), control_lines.end() );

        if ( num_controls <= (unsigned)ceil( circ.lines() / 2.0 ) && num_controls > 3u ) // Bugfix: use 7.3 decomp when more than one aux. line
        {
          // decomposition Barenco Lemma 7.2
          unsigned needed_empty_lines = num_controls - 2;

          // prepare the gates
          std::vector<gate> e_gates; // gates on empty lines
          std::vector<gate> e_gates_inv; // inverse gates
          unsigned target = 0;
          for ( unsigned i = 0; i < needed_empty_lines; ++i )
          {
            target = ( i == ( needed_empty_lines - 1 ) ) ? *g.begin_targets() : empty_lines.at( i + 1 );

            gate ng;
            ng.set_type( peres_tag() );
            ng.add_control( control_lines.at( 2 + i ) );
            ng.add_target( empty_lines.at( i ) );
            ng.add_target( target );
            e_gates += ng;
            ng.set_type( inverse_peres_tag() );
            e_gates_inv += ng;
          }

          gate middle;
          middle.set_type( peres_tag() );
          middle.add_control( control_lines.at( 0 ) );
          middle.add_target( control_lines.at( 1 ) );
          middle.add_target( empty_lines.at( 0 ) );

          gate middle_inv = middle;
          middle_inv.set_type( inverse_peres_tag() );

          // add the gates
          for ( int i = needed_empty_lines - 1; i >= 0; --i )
          {
            operator()( circ, e_gates.at( i ) );
          }

          operator()( circ, middle );

          for ( unsigned i = 0; i < needed_empty_lines; ++i )
          {
            operator()( circ, e_gates.at( i ) );
          }

          // again without target
          for ( int i = needed_empty_lines - 2; i >= 0; --i )
          {
            operator()( circ, e_gates.at( i ) );
          }

          operator()( circ, middle );

          for ( unsigned i = 0; i < needed_empty_lines - 1; ++i )
          {
            operator()( circ, e_gates.at( i ) );
          }
        }
        else
        {
          unsigned first_controls = (unsigned)ceil( circ.lines() / 2.0 );
          if ( num_controls == 3u ) // Bugfix (see above)
          {
            first_controls = 2u;
          }

          // decomposition Barenco Lemma 7.3
          gate g1;
          g1.set_type( toffoli_tag() );
          for ( unsigned i = 0; i < first_controls; ++i )
          {
            g1.add_control( control_lines.at( i ) );
          }
          g1.add_target( empty_lines.front() );

          gate g2;
          g2.set_type( toffoli_tag() );
          for ( unsigned i = first_controls; i < num_controls; ++i )
          {
            g2.add_control( control_lines.at( i ) );
          }
          g2.add_control( empty_lines.front() );
          g2.add_target( *g.begin_targets() );

          // NOTE optimize by doing it once for g1 and g2 and then copy
          operator()( circ, g1 );
          operator()( circ, g2 );
          operator()( circ, g1 );
          operator()( circ, g2 );
        } 
      }
    }
    else if ( is_peres( g ) )
    {
      unsigned c = *g.begin_controls();
      gate::const_iterator it = g.begin_targets();
      unsigned t1 = *it++;
      unsigned t2 = *it;
      
      append_v( circ, t1, t2 );
      append_v( circ, c, t2 );
      append_cnot( circ, c, t1 );
      append_vplus( circ, t1, t2 );
    }
    else if ( is_inverse_peres( g ) )
    {
      unsigned c = *g.begin_controls();
      gate::const_iterator it = g.begin_targets();
      unsigned t1 = *it++;
      unsigned t2 = *it;
      
      append_v( circ, t1, t2 );
      append_cnot( circ, c, t1 );
      append_vplus( circ, c, t2 );
      append_vplus( circ, t1, t2 );
    }
    else
    {
      assert( false );
    }
  }

  bool quantum_decomposition( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    std::string helper_line_input              = get<std::string>( settings, "helper_line_input", "w" );
    std::string helper_line_output             = get<std::string>( settings, "helper_line_output", "w" );
    gate_decomposition_func gate_decomposition = get<gate_decomposition_func>( settings, "gate_decomposition", standard_decomposition() );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    clear_circuit( circ );
    copy_metadata( base, circ );

    // check whether an additional work line is required
    bool add_line = false;
    if ( base.lines() > 3 )
    {
      foreach ( const gate& g, base )
      {
        if ( std::distance( g.begin_controls(), g.end_controls() ) == (int)( base.lines() - 1 ) )
        {
          add_line = true;
          break;
        }
      }
    }

    if ( add_line )
    {
      add_line_to_circuit( circ, helper_line_input, helper_line_output, false, true );
    }

    foreach ( const gate& g, base )
    {
      gate_decomposition( circ, g);
    }

    return true;
  }

  decomposition_func quantum_decomposition_func( properties::ptr settings, properties::ptr statistics )
  {
    decomposition_func f = boost::bind( &quantum_decomposition, _1, _2, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}
