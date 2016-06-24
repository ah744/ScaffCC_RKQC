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

#include "sequential_simulation.hpp"

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <core/functor.hpp>
#include <core/utils/timer.hpp>

#include <algorithms/simulation/partial_simulation.hpp>

using namespace boost::assign;

namespace revkit
{

  // If needed somewhere else, put to utilities of simulation
  class wave_file
  {
  public:
    wave_file( const std::string& filename, const std::string& name )
      : wire_count( 0u ),
        time_step( 0u )
    {
      os.open( filename.c_str() );

      os << "$timescale 1 ps $end" << std::endl << std::endl;

      os << "$scope module " << name << " $end" << std::endl;
      os << "$var wire 1 clk clock $end" << std::endl;
    }

    bool add_wire( const std::string& name, unsigned num_bits = 1u )
    {
      if ( wire_mapping.find( name ) != wire_mapping.end() )
      {
        std::cout << "Wire with name " << name << " has been defined already" << std::endl;
        return false;
      }

      wire_mapping.insert( std::make_pair( name, wire_count ) );

      os << "$var wire " << num_bits << " w" << wire_count << " " << name << " $end" << std::endl;

      ++wire_count;

      return true;
    }

    bool add_signal( const std::string& name, bool value )
    {
      std::map<std::string, unsigned>::const_iterator it = wire_mapping.find( name );

      if ( it == wire_mapping.end() )
      {
        return false;
      }

      os << value << "w" << it->second << std::endl;

      return true;
    }

    bool add_signal( const std::string& name, const boost::dynamic_bitset<>& value )
    {
      std::map<std::string, unsigned>::const_iterator it = wire_mapping.find( name );

      if ( it == wire_mapping.end() )
      {
        return false;
      }

      os << "b" << value << " w" << it->second << std::endl;

      return true;
    }

    void next()
    {
      if ( time_step == 0u )
      {
        os << "$upscope $end" << std::endl;
        os << "$enddefinitions $end" << std::endl << std::endl;
      }

      os << boost::format( "#%d" ) % ( time_step * 1000u ) << std::endl;
      os << boost::format( "%dclk" ) % ( time_step % 2u ) << std::endl;

      ++time_step;
    }

    void close()
    {
      os << boost::format( "#%d" ) % ( time_step * 1000u ) << std::endl;
    }

  private:
    std::ofstream os;
    unsigned wire_count;
    std::map<std::string, unsigned> wire_mapping;
    unsigned time_step;
  };

  template<typename Integer>
  boost::iterator_range<boost::range_detail::integer_iterator<Integer> >
  irange( Integer last )
  {
    return boost::irange( 0u, last );
  }

  // Maybe needed somewhere else as well
  // Input at i is no constant and no state variable and in no bus
  inline bool is_single_input( const circuit& circ, unsigned index )
  {
    return !circ.constants().at( index ) &&
      !circ.statesignals().has_bus( index ) &&
      !circ.inputbuses().has_bus( index );
  }

  // Maybe needed somewhere else as well
  // Output at i is no garbage and no state variable and in no bus
  inline bool is_single_output( const circuit& circ, unsigned index )
  {
    return !circ.garbage().at( index ) &&
      !circ.statesignals().has_bus( index ) &&
      !circ.outputbuses().has_bus( index );
  }

  inline bool is_input( const circuit& circ, unsigned index )
  {
    return !circ.constants().at( index );
  }

  inline bool is_output( const circuit& circ, unsigned index )
  {
    return !circ.garbage().at( index );
  }

  inline bool is_primary_input( const circuit& circ, unsigned index )
  {
    return is_input( circ, index ) && !circ.statesignals().has_bus( index );
  }

  inline bool is_primary_output( const circuit& circ, unsigned index )
  {
    return is_output( circ, index ) && !circ.statesignals().has_bus( index );
  }

  bool sequential_simulation( std::vector<boost::dynamic_bitset<> >& outputs,
                              const circuit& circ,
                              const std::vector<boost::dynamic_bitset<> >& inputs,
                              properties::ptr settings, properties::ptr statistics )
  {

    typedef std::map<std::string, boost::dynamic_bitset<> > bitset_map;

    // Settings parsing
    bitset_map initial_state = get<bitset_map>( settings, "initial_state", bitset_map() );
    std::string vcd_filename = get<std::string>( settings, "vcd_filename", std::string() );
    sequential_step_result_func step_result = get<sequential_step_result_func>( settings, "step_result", sequential_step_result_func() );

    // Run-time measuring
    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    // Wavefile Generation
    bool write_wavefile = !vcd_filename.empty();
    boost::shared_ptr<wave_file> vcd_file;

    bitset_map ibitsets;
    bitset_map sbitsets;
    bitset_map obitsets;

    foreach ( const bus_collection::map::value_type& bus, circ.inputbuses().buses() )
    {
      ibitsets.insert( std::make_pair( bus.first, boost::dynamic_bitset<>( bus.second.size() ) ) );
    }

    foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
    {
      sbitsets.insert( std::make_pair( bus.first, boost::dynamic_bitset<>( bus.second.size() ) ) );

      bitset_map::const_iterator itInitialState = initial_state.find( bus.first );
      if ( itInitialState != initial_state.end() )
      {
        // found a key defined in the initial states
        // now the respective bitvectors have to match in size
        if ( itInitialState->second.size() == bus.second.size() )
        {
          // ok, sizes match
          sbitsets[bus.first] = itInitialState->second;
        }
        else
        {
          // failed
          set_error_message( statistics, boost::str( boost::format( "Bit-vector of initial state %s has wrong bit-width %d. Expected bitwidth is %d." ) % bus.first % itInitialState->second % bus.second.size() ) );
          return false;
        }
      }
    }

    foreach ( const bus_collection::map::value_type& bus, circ.outputbuses().buses() )
    {
      obitsets.insert( std::make_pair( bus.first, boost::dynamic_bitset<>( bus.second.size() ) ) );
    }

    // wave file
    if ( write_wavefile )
    {
      vcd_file.reset( new wave_file( vcd_filename, "RevKit" ) );

      // inputs
      foreach ( const bus_collection::map::value_type& bus, circ.inputbuses().buses() )
      {
        vcd_file->add_wire( bus.first, bus.second.size() );
      }

      foreach ( unsigned index, irange( circ.lines() ) )
      {
        if ( is_single_input( circ, index ) )
        {
          vcd_file->add_wire( circ.inputs().at( index ) );
        }
      }

      // states
      foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
      {
        vcd_file->add_wire( bus.first, bus.second.size() );
        vcd_file->add_wire( bus.first + "'", bus.second.size() );
      }

      // outputs
      foreach ( const bus_collection::map::value_type& bus, circ.outputbuses().buses() )
      {
        vcd_file->add_wire( bus.first + "'", bus.second.size() );
      }

      foreach ( unsigned index, irange( circ.lines() ) )
      {
        if ( is_single_output( circ, index ) )
        {
          vcd_file->add_wire( circ.outputs().at( index ) + "'" );
        }
      }
    }

    // number of inputs (no constants)
    unsigned num_inputs = boost::count_if( irange( circ.lines() ), boost::bind( is_input, circ, _1 ) );
    // number of primary outputs (no garbage)
    unsigned num_pos = boost::count_if( irange( circ.lines() ), boost::bind( is_primary_output, circ, _1 ) );

    // simulate
    unsigned current_input = 0u;
    boost::dynamic_bitset<> next_input( 0u ); // can be given by the step_result_func, however, current_input has priority
    while ( current_input < inputs.size() || !next_input.empty() )
    {
      using boost::adaptors::filtered;

      boost::dynamic_bitset<> input_assignment = ( current_input < inputs.size() ? inputs.at( current_input ) : next_input );

      // wave file
      if ( write_wavefile )
      {
        vcd_file->next();
      }

      boost::dynamic_bitset<> input( num_inputs );
      unsigned pos = 0u;
      unsigned pos_in_input = 0u;

      foreach ( unsigned index, irange( circ.lines() ) | filtered( boost::bind( is_input, circ, _1 ) ) )
      {
        // PI or PPI?
        if ( is_primary_input( circ, index ) ) // PI
        {
          input.set( pos, input_assignment.test( pos_in_input++ ) );

          // wave file
          if ( write_wavefile )
          {
            std::string bus_name = circ.inputbuses().find_bus( index );
            if ( !bus_name.empty() )
            {
              const std::vector<unsigned>& line_indices = circ.inputbuses().get( bus_name );
              ibitsets[bus_name].set( std::find( line_indices.begin(), line_indices.end(), index ) - line_indices.begin(), input.test( pos ) );
            }
            else
            {
              vcd_file->add_signal( circ.inputs().at( index ), input.test( pos ) );
            }

          }
        }
        else // PPI
        {
          input.set( pos, sbitsets[circ.statesignals().find_bus( index )].test( circ.statesignals().signal_index( index ) ) );
        }

        ++pos;
      }

      // wave file
      if ( write_wavefile )
      {
        for ( bitset_map::const_iterator itBitset = ibitsets.begin(); itBitset != ibitsets.end(); ++itBitset )
        {
          vcd_file->add_signal( itBitset->first, itBitset->second );
        }

        for ( bitset_map::const_iterator itBitset = sbitsets.begin(); itBitset != sbitsets.end(); ++itBitset )
        {
          vcd_file->add_signal( itBitset->first, itBitset->second );
        }
      }

      boost::dynamic_bitset<> output;
      partial_simulation( output, circ, input );

      boost::dynamic_bitset<> outputAssignment( num_pos );

      // reset obitsets
      for ( bitset_map::iterator itBitset = obitsets.begin(); itBitset != obitsets.end(); ++itBitset )
      {
        itBitset->second.reset();
      }

      pos = 0u;
      unsigned pos_in_output = 0u;
      foreach ( unsigned index, irange( circ.lines() ) | filtered( boost::bind( is_output, circ, _1 ) ) )
      {
        // PO or PPO?
        if ( is_primary_output( circ, index ) ) // PO
        {
          outputAssignment.set( pos_in_output++, output.test( pos ) );

          // wave file
          if ( write_wavefile )
          {
            std::string bus_name = circ.outputbuses().find_bus( index );
            if ( !bus_name.empty() )
            {
              const std::vector<unsigned>& line_indices = circ.outputbuses().get( bus_name );
              obitsets[bus_name].set( std::find( line_indices.begin(), line_indices.end(), index ) - line_indices.begin(), output.test( pos ) );
            }
            else
            {
              vcd_file->add_signal( circ.outputs().at( index ) + "'", output.test( pos ) );
            }
          }
        }
        else
        {
          sbitsets[circ.statesignals().find_bus( index )].set( circ.statesignals().signal_index( index ), output.test( pos ) );
        }
        ++pos;
      }

      // wave file
      if ( write_wavefile )
      {
        for ( bitset_map::const_iterator itBitset = obitsets.begin(); itBitset != obitsets.end(); ++itBitset )
        {
          vcd_file->add_signal( itBitset->first + "'", itBitset->second );
        }

        for ( bitset_map::const_iterator itBitset = sbitsets.begin(); itBitset != sbitsets.end(); ++itBitset )
        {
          vcd_file->add_signal( itBitset->first + "'", itBitset->second );
        }
      }

      outputs += outputAssignment;

      if ( step_result )
      {
        next_input = step_result( sbitsets, outputAssignment );
      }

      ++current_input;
    }

    if ( write_wavefile )
    {
      vcd_file->close();
    }

    return true;
  }

  multi_step_simulation_func sequential_simulation_func( properties::ptr settings, properties::ptr statistics )
  {
    multi_step_simulation_func f = boost::bind( sequential_simulation, _1, _2, _3, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}
