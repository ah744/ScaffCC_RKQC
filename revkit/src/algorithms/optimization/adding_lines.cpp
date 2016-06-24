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

#include "adding_lines.hpp"

#include <boost/bind.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <core/functor.hpp>
#include <core/target_tags.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/add_line_to_circuit.hpp>
#include <core/functions/copy_circuit.hpp>
#include <core/functions/copy_metadata.hpp>
#include <core/utils/costs.hpp>
#include <core/utils/timer.hpp>

#define foreach BOOST_FOREACH

namespace revkit
{

  template<typename Iterator, typename OutputIterator>
  OutputIterator _make_factor( Iterator first, Iterator last, const boost::dynamic_bitset<>& factor, OutputIterator output )
  {
    for ( Iterator it = first; it != last; ++it )
    {
      if ( factor.test( it.index() ) )
      {
        *output++ = *it;
      }
    }

    return output;
  }

  template<typename SinglePassRange, typename OutputIterator>
  OutputIterator _make_factor( const SinglePassRange& range, const boost::dynamic_bitset<>& factor, OutputIterator output )
  {
    return _make_factor( boost::begin( range ), boost::end( range ), factor, output );
  }

  template<typename SinglePassRange, typename OutputContainer>
  void make_factor( const SinglePassRange& range, const boost::dynamic_bitset<>& factor, OutputContainer& output )
  {
    _make_factor( range | boost::adaptors::indexed( 0u ), factor, std::insert_iterator<OutputContainer>( output, output.begin() ) );
  }

  unsigned find_suitable_gates( const circuit& base, unsigned index, const gate::line_container& factor )
  {
    // last gate?
    if ( index == base.num_gates() )
    {
      return index;
    }

    // only Toffoli gates
    if ( !is_toffoli( base[index] ) )
    {
      return index;
    }

    // NOTE check for helper line?
    // has target in controls?
    if ( boost::find( factor, *base[index].begin_targets() ) != factor.end() )
    {
      return index;
    }

    // check next gate
    return find_suitable_gates( base, index + 1, factor );
  }

  int calculate_cost_reduction( const circuit& base, unsigned start, unsigned end, const gate::line_container& factor, unsigned helper_line )
  {
    circuit tmp;
    copy_metadata( base, tmp );

    /* original costs */
    for ( circuit::const_iterator it = base.begin() + start; it != base.begin() + end; ++it )
    {
      tmp.append_gate() = *it;
    }

    unsigned original_costs = costs( tmp, costs_by_gate_func( quantum_costs() ) );

    /* modify circuit */
    foreach ( gate& g, tmp )
    {
      gate::line_container controls( g.begin_controls(), g.end_controls() );
      if ( !boost::includes( controls, factor ) ) continue;

      g.add_control( helper_line );
      foreach ( unsigned control, factor )
      {
        g.remove_control( control );
      }
    }
    prepend_toffoli( tmp, factor, helper_line );
    append_toffoli( tmp, factor, helper_line );

    unsigned new_costs = costs( tmp, costs_by_gate_func( quantum_costs() ) );

    return original_costs - new_costs;
  }

  bool adding_lines( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {

    // Settings parsing
    unsigned additional_lines = get<unsigned>( settings, "additional_lines", 1 );

    // Run-time measuring
    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    // copy circuit
    copy_circuit( base, circ );

    for ( unsigned h = 0u; h < additional_lines; ++h )
    {
      /* add one helper line */
      unsigned helper_line = add_line_to_circuit( circ, "helper", "helper", false, true );

      /* last inserted helper gate (to be removed in the end) */
      unsigned last_helper_gate_index = 0u;

      unsigned current_index = 0u;
      while ( current_index < circ.num_gates() )
      {
        /* best factor */
        unsigned best_cost_reduction = 0u;
        unsigned best_factor = 0u;
        unsigned best_j = 0u;

        /* control lines of the current gate */
        const gate& current_gate = circ[current_index];
        gate::line_container controls( current_gate.begin_controls(), current_gate.end_controls() );

        /* generate each factor */
        foreach ( unsigned F, boost::irange( 1u, 1u << controls.size() ) )
        {
          boost::dynamic_bitset<> factor( controls.size(), F );
          if ( factor.count() <= 1u ) continue; /* only consider factors of size > 1 */

          /* create factor */
          gate::line_container factored;
          make_factor( controls, factor, factored );

          /* determine upper bound */
          unsigned j = find_suitable_gates( circ, current_index, factored );

          /* calculate cost reduction */
          int cost_reduction = calculate_cost_reduction( circ, current_index, j, factored, helper_line );

          /* better? */
          if ( cost_reduction > (int)best_cost_reduction )
          {
            best_cost_reduction = (unsigned)cost_reduction; /* in this case cost_reduction is greater than 0 */
            best_factor = F;
            best_j = j;
          }
        }
      
        /* was a factor found? */
        if ( best_factor != 0u )
        {
          gate::line_container factored;
          make_factor( controls, boost::dynamic_bitset<>( controls.size(), best_factor), factored );

          /* apply factor */
          foreach ( unsigned i, boost::irange( current_index, best_j ) )
          {
            gate::line_container controls( circ[i].begin_controls(), circ[i].end_controls() );
            if ( !boost::includes( controls, factored ) ) continue;

            circ[i].add_control( helper_line );
            foreach ( unsigned control, factored )
            {
              circ[i].remove_control( control );
            }
          }

          /* toffoli gate at the beginning */
          insert_toffoli( circ, current_index, factored, helper_line );
        
          /* update best_j, since we inserted a gate before */
          ++best_j;

          /* toffoli gate at the end */
          insert_toffoli( circ, best_j, factored, helper_line );
          last_helper_gate_index = best_j;

          /* update again */
          ++best_j;

          /* update current_index */
          current_index = best_j;
        }
        else
        {
          /* check next gate */
          ++current_index;
        }
      }

      /* remove last helper gate if added */
      if ( last_helper_gate_index != 0u )
      {
        circ.remove_gate_at( last_helper_gate_index );
      }
    }

    return true;
  }

  optimization_func adding_lines_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = boost::bind( adding_lines, _1, _2, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}
