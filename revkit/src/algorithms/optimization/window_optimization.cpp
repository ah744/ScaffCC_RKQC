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

#include "window_optimization.hpp"

#include <boost/bind.hpp>

#include <core/functions/add_circuit.hpp>
#include <core/functions/circuit_to_truth_table.hpp>
#include <core/functions/copy_circuit.hpp>
#include <core/functions/expand_circuit.hpp>
#include <core/io/print_circuit.hpp>
#include <core/utils/timer.hpp>

#include <algorithms/simulation/simple_simulation.hpp>
#include <algorithms/synthesis/transformation_based_synthesis.hpp>

namespace revkit
{

  shift_window_selection::shift_window_selection()
    : window_length( 10u ),
      offset( 1u ),
      pos( 0u )
  {
  }

  circuit shift_window_selection::operator()( const circuit& base )
  {
    if ( pos >= base.num_gates() )
    {
      /* dont forget to reset in case of second call */
      pos = 0u;
      return circuit();
    }

    unsigned length = std::min( window_length, base.num_gates() - pos );
    unsigned to = pos + length;

    subcircuit s( base, pos, to );
    pos += offset;

    return s;
  }

  line_window_selection::line_window_selection()
    : num_lines( 0u ),
      line_count( 2u ),
      pos( 0u )
  {
  }

  circuit line_window_selection::operator()( const circuit& base )
  {
    /* set number of lines */
    if ( !num_lines )
    {
      num_lines = base.lines();
    }

    while ( true )
    {
      /* increment line_count or reset and finish */
      if ( pos >= base.num_gates() )
      {
        pos = 0u;

        if ( line_count < num_lines - 1u )
        {
          ++line_count;
        }
        else
        {
          line_count = 2u;
          return circuit();
        }
      }

      unsigned start_pos = pos;
      gate::line_container non_empty_lines;

      for ( unsigned i = pos; i < base.num_gates(); ++i )
      {
        gate::line_container new_non_empty_lines;

        /* copy controls and targets */
        const gate& g = *( base.begin() + i );
        std::copy( g.begin_controls(), g.end_controls(), std::insert_iterator<gate::line_container>( new_non_empty_lines, new_non_empty_lines.begin() ) );
        std::copy( g.begin_targets(), g.end_targets(), std::insert_iterator<gate::line_container>( new_non_empty_lines, new_non_empty_lines.begin() ) );
        std::copy( non_empty_lines.begin(), non_empty_lines.end(), std::insert_iterator<gate::line_container>( new_non_empty_lines, new_non_empty_lines.begin() ) );

        if ( new_non_empty_lines.size() <= line_count )
        {
          /* keep on trying */
          non_empty_lines.clear();
          std::copy( new_non_empty_lines.begin(), new_non_empty_lines.end(), std::insert_iterator<gate::line_container>( non_empty_lines, non_empty_lines.begin() ) );
        }
        else
        {
          /* do we have a circuit for now? */
          if ( non_empty_lines.size() )
          {
            pos = i;
            std::vector<unsigned> filter( non_empty_lines.begin(), non_empty_lines.end() );
            return subcircuit( base, start_pos, pos, filter );
          }
          else
          {
            /* new start pos */
            start_pos = i + 1;
          }
        }
      }

      /* still here? */
      pos = base.num_gates();

      /* do we have a circuit? */
      if ( non_empty_lines.size() )
      {
        std::vector<unsigned> filter( non_empty_lines.begin(), non_empty_lines.end() );
        return subcircuit( base, start_pos, base.num_gates(), filter );
      }
      else
      {
        /* next iteration */
      }
    }
  }

  resynthesis_optimization::resynthesis_optimization()
    : synthesis( transformation_based_synthesis_func() ),
      simulation( simple_simulation_func() )
  {
  }

  bool resynthesis_optimization::operator()( circuit& new_window, const circuit& old_window ) const
  {
    binary_truth_table spec;
    circuit_to_truth_table( old_window, spec, simulation );
    return synthesis( new_window, spec );
  }

  bool window_optimization( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    select_window_func select_window = get<select_window_func>( settings, "select_window", shift_window_selection() );
    optimization_func  optimization  = get<optimization_func>( settings, "optimization", resynthesis_optimization() );
    cost_function cf = get<cost_function>( settings, "cost_function", costs_by_circuit_func( gate_costs() ) );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    copy_circuit( base, circ );

    while ( true )
    {
      /* select the window */
      circuit s = select_window( circ );

      /* check if window is still valid */
      if ( s.num_gates() == 0 )
      {
        break;
      }

      /* obtain the new window */
      circuit new_window;
      bool ok = optimization( new_window, s );

      /* check if it is cheaper */
      bool cheaper = ok && costs( new_window, cf ) < costs( s, cf );

      if ( cheaper )
      {
        /* remove old sub-circuit */
        unsigned s_size = s.num_gates(); // save in variable since we are changing its base
        unsigned s_from = s.offset();
        for ( unsigned i = 0u; i < s_size; ++i )
        {
          circ.remove_gate_at( s_from );
        }

        circuit window_expanded;
        expand_circuit( new_window, window_expanded, circ.lines(), s.filter().second );
        insert_circuit( circ, s_from, window_expanded );
      }
    }

    return true;
  }

  optimization_func window_optimization_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = boost::bind( window_optimization, _1, _2, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}
