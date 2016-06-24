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

#include "partial_simulation.hpp"

#include <boost/bind.hpp>

#include <core/circuit.hpp>
#include <core/gate.hpp>

#include <core/utils/timer.hpp>

#include <algorithms/simulation/simple_simulation.hpp>

namespace revkit
{

  bool partial_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input, properties::ptr settings, properties::ptr statistics )
  {
    simulation_func simulation = get<simulation_func>( settings, "simulation", simple_simulation_func() );
    bool keep_full_output = get<bool>( settings, "keep_full_output", false );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    boost::dynamic_bitset<> full_input( circ.lines(), 0ul );
    
    unsigned input_pos = 0u;

    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      constant con = circ.constants().at( i );

      full_input.set( i, con ? *con : input.test( input_pos ) );
      if ( !con )
      {
        ++input_pos;
      }
    }

    boost::dynamic_bitset<> full_output;

    simulation( full_output, circ, full_input );

    if ( keep_full_output )
    {
      output = full_output;
    }
    else
    {
      output.resize( std::count( circ.garbage().begin(), circ.garbage().end(), false ) );

      unsigned output_pos = 0u;
      for ( unsigned i = 0; i < full_output.size(); ++i )
      {
        if ( !circ.garbage().at( i ) )
        {
          output.set( output_pos++, full_output.test( i ) );
        }
      }
    }

    return true;
  }

  simulation_func partial_simulation_func( properties::ptr settings, properties::ptr statistics )
  {
    simulation_func f = boost::bind( static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, properties::ptr, properties::ptr)>( partial_simulation ), _1, _2, _3, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}

