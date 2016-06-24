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

#include "line_reduction.hpp"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>

#include <core/circuit.hpp>
#include <core/truth_table.hpp>
#include <core/functions/add_circuit.hpp>
#include <core/functions/copy_circuit.hpp>
#include <core/functions/expand_circuit.hpp>
#include <core/functions/find_lines.hpp>
#include <core/io/print_circuit.hpp>
#include <core/io/write_realization.hpp>
#include <core/io/read_realization.hpp>
#include <core/utils/timer.hpp>

#include <algorithms/simulation/partial_simulation.hpp>
#include <algorithms/simulation/simple_simulation.hpp>
#include <algorithms/synthesis/embed_truth_table.hpp>
#include <algorithms/synthesis/transformation_based_synthesis.hpp>

//#include <unstable/synthesis/sword_synthesis.hpp>

using namespace boost::assign;

namespace revkit
{
#ifndef __WIN32
  template<typename Func>
  bool timed_synthesis( circuit& circ, Func& func, binary_truth_table const& spec, unsigned timeout )
  {
    if ( timeout == 0u )
    {
      return func( circ, spec );
    }

    char name[] =  "revkit_synthesis_XXXXXXX";
    char *tmpName = mktemp( name );

    if ( !tmpName )
    {
      return false;
    }

    pid_t pid = fork ();

    if ( pid == 0 )
    {
      struct rlimit r;
      r.rlim_cur = r.rlim_max = timeout;
      setrlimit( RLIMIT_CPU, &r );

      bool result = func( circ, spec );

      if ( result )
      {
        write_realization( circ, tmpName );
      }

      _exit( 0 );
    }
    else
    {
      int status;
      pid_t w;

      do {
        w = waitpid( pid, &status, 0 );

        if ( w == -1 )
        {
          perror( "waitpid" );
          exit( EXIT_FAILURE );
        }

        if ( WIFEXITED( status ) ) {
        } else if ( WIFSIGNALED( status ) ) {
        } else if ( WIFSTOPPED( status ) ) {
        } else if ( WIFCONTINUED( status ) ) {
        }
      } while ( !WIFEXITED( status ) && !WIFSIGNALED( status ) );

      circuit newCirc;
      if ( !read_realization( newCirc, tmpName ) )
        return false;
      else
        copy_circuit( newCirc, circ );

      remove( tmpName );

      return true;
    }

    assert ( false && "This should never be reached." );
  }
#endif


  struct has_control_at
  {
    explicit has_control_at( unsigned i ) : _i( i ) {}

    bool operator()( const gate& g ) const
    {
      return std::find( g.begin_controls(), g.end_controls(), _i ) != g.end_controls();
    }

  private:
    unsigned _i;
  };

  struct has_control_or_target_at
  {
    explicit has_control_or_target_at( unsigned i ) : _i( i ) {}

    bool operator()( const gate& g ) const
    {
      return std::find( g.begin_controls(), g.end_controls(), _i ) != g.end_controls() || std::find( g.begin_targets(), g.end_targets(), _i ) != g.end_targets();
    }

  private:
    unsigned _i;
  };

  //// embed_and_synthesize ////

  embed_and_synthesize::embed_and_synthesize()
    : embedding( embed_truth_table_func() )
     , synthesis( transformation_based_synthesis_func() )
     , timeout ( 0u )
  {
  }

  bool embed_and_synthesize::operator()( circuit& circ, binary_truth_table& spec, const std::vector<unsigned>& output_order )
  {
    embedding.settings()->set( "output_order", output_order );
    if ( !embedding( spec, spec ) )
    {
      return false;
    }

#ifdef __WIN32
    return synthesis( circ, spec );
#else
    return timed_synthesis( circ, synthesis, spec, timeout );
#endif
  }

  /*
   * We assume that the circuit is optimized by preprocessing
   * and that each garbage line has at least one control as
   * last line type
   */
  unsigned find_best_garbage_line( const circuit& circ, const std::vector<unsigned>& lines_to_skip, const std::vector<unsigned>& original_lines, unsigned& last_control_position )
  {
    std::map<unsigned, unsigned> garbage_to_last_control;

    for ( unsigned i = 0u; i < circ.lines(); ++i ) {
      if ( circ.garbage().at( i ) && std::find( lines_to_skip.begin(), lines_to_skip.end(), original_lines.at( i ) ) == lines_to_skip.end() ) {
        circuit::const_reverse_iterator itPosition = std::find_if( circ.rbegin(), circ.rend(), has_control_at( i ) );

        /* unoptimized circuit? */
        if ( itPosition == circ.rend() )
        {
          continue;
        }

        unsigned position = circ.rend() - 1 - itPosition;

        garbage_to_last_control.insert( std::make_pair( i, position ) );
      }
    }

    if ( !garbage_to_last_control.size() ) {
      last_control_position = 0u;
      return circ.lines();
    } else {
      std::map<unsigned, unsigned>::const_iterator itMin = std::min_element( garbage_to_last_control.begin(), garbage_to_last_control.end(),
                                                                             boost::bind( std::less<unsigned>(),
                                                                                          boost::bind( &std::map<unsigned,unsigned>::value_type::second, _1 ),
                                                                                          boost::bind( &std::map<unsigned,unsigned>::value_type::second, _2 ) ) );
      last_control_position = itMin->second;
      return itMin->first;
    }
  }

  unsigned num_non_empty_lines( const circuit& circ, unsigned from, unsigned length )
  {
    gate::line_container c;
    find_non_empty_lines( circ.begin() + from, circ.begin() + from + length, std::insert_iterator<gate::line_container>( c, c.begin() ) );
    return c.size();
  }

  circuit find_window_with_max_lines( const circuit& circ, unsigned end, unsigned max_lines )
  {
    unsigned start = end;

    while ( num_non_empty_lines( circ, start, end - start + 1 ) <= max_lines && start > 0 ) {
      --start;
    }

    if ( num_non_empty_lines( circ, start, end - start + 1 ) > max_lines ) {
      ++start;
    }

    std::vector<unsigned> filter;
    find_non_empty_lines( circ.begin() + start, circ.begin() + end + 1, std::back_inserter( filter ) );
    return subcircuit( circ, start, end + 1, filter );
  }

  unsigned find_constant_line( const circuit& circ, unsigned window_end )
  {
    unsigned best_line = circ.lines();
    unsigned min_gate_index = circ.num_gates();

    // go through all constants
    for ( std::vector<constant>::const_iterator it = circ.constants().begin(); it != circ.constants().end(); ++it )
    {
      // if constant
      if ( *it )
      {
        unsigned index = it - circ.constants().begin();

        // if line is empty until window_end
        if ( std::find_if( circ.begin(), circ.begin() + window_end, has_control_or_target_at( index ) ) == circ.begin() + window_end )
        {
          unsigned gate_index = std::find_if( circ.begin(), circ.end(), has_control_or_target_at( index ) ) - circ.begin();
          if ( gate_index >= circ.num_gates() ) // this would imply an empty line, optimization
          {
            continue;
          }

          if ( gate_index < min_gate_index )
          {
            best_line = index;
            min_gate_index = gate_index;
          }
        }
      }
    }

    return best_line;
  }

  /* returns a set of the function of the line, which is 0,1 if it is supposed to be the constant line,
     2 if it needs to be used afterwards, or -1 if it is not needed anymore. */
  void garbage_to_ov( const circuit& circ, const circuit& window, const std::vector<unsigned>& line_mapping,
                      unsigned garbage_line, std::vector<short>& ov, bool constant_value )
  {
    for ( unsigned i = 0u; i < window.lines(); ++i )
    {
      unsigned mapped_line = line_mapping.at( i );

      if ( mapped_line == garbage_line )
      {
        ov += constant_value ? 1 : 0;
      }
      else if ( !circ.garbage().at( mapped_line ) )
      {
        ov += 2;
      }
      else
      {
        if ( std::find_if( circ.begin() + window.offset() + window.num_gates(), circ.end(), has_control_or_target_at( mapped_line ) ) == circ.end() )
        {
          ov += -1;
        }
        else
        {
          ov += 2;
        }
      }
    }
  }

  bool create_window_specification( const circuit& window, binary_truth_table& window_spec, const std::vector<unsigned long long>& assignments, const std::vector<short>& ov,
                                    const simulation_func& simulation )
  {
    bool needs_simulation = std::find( ov.begin(), ov.end(), 2 ) != ov.end();
    unsigned num_dcs = std::count( ov.begin(), ov.end(), -1 );

    std::map<binary_truth_table::cube_type, unsigned> output_assignments;

    for ( unsigned long long i = 0ull; i < ( 1ull << window.lines() ); ++i )
    {
      // output cube
      if ( std::find( assignments.begin(), assignments.end(), i ) != assignments.end() ) // input has to be simulated
      {
        binary_truth_table::cube_type input_cube, output_cube;

        boost::dynamic_bitset<> simulation_input( window.lines(), i ), simulation_result;
        if ( needs_simulation )
        {
          simulation( simulation_result, window, simulation_input );
        }

        for ( std::vector<short>::const_iterator itOV = ov.begin(); itOV != ov.end(); ++itOV )
        {
          switch ( *itOV ) {
          case -1:
            break;
          case 0:
          case 1:
            output_cube += constant( *itOV == 1 );
            break;
          case 2:
          default:
            output_cube += constant( simulation_result.test( itOV - ov.begin() ) );
            break;
          }
        }

        // input cube
        for ( unsigned j = 0u; j < window.lines(); ++j )
        {
          input_cube.push_back( i & ( 1u << j ) );
        }

        window_spec.add_entry( input_cube, output_cube );

        if ( output_assignments.find( output_cube ) == output_assignments.end() )
        {
          output_assignments.insert( std::make_pair( output_cube, 1u ) );
        }
        else
        {
          if ( output_assignments[output_cube] >= ( 1u << num_dcs ) )
          {
            window_spec.clear();
            return false;
          }
          ++output_assignments[output_cube];
        }
      }
    }

    return true;
  }

  void ov_to_order_vector( const std::vector<short>& ov, std::vector<unsigned>& order )
  {
    order.clear();

    for ( std::vector<short>::const_iterator it = ov.begin(); it != ov.end(); ++it )
    {
      if ( *it != -1 )
      {
        order += ( it - ov.begin() );
      }
    }
  }

  /* removes the line line_to_remove and moves all the items on that line to line_to_use */
  void remove_line( circuit& circ, unsigned line_to_remove, unsigned line_to_use )
  {
    if ( line_to_use > line_to_remove )
    {
      --line_to_use;
    }

    foreach ( gate& g, circ )
    {
      // NOTE make it really possible to change the gate iterator
      gate::line_container c;
      for ( gate::iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl )
      {
        if ( *itControl >= line_to_remove )
        {
          c += *itControl;
        }
      }

      foreach ( const unsigned& control, c )
      {
        if ( control == line_to_remove )
        {
          g.remove_control( control );
          g.add_control( line_to_use );
        }
        else
        {
          g.remove_control( control );
          g.add_control( control - 1 );
        }
      }

      c.clear();
      for ( gate::iterator itTarget = g.begin_targets(); itTarget != g.end_targets(); ++itTarget )
      {
        if ( *itTarget >= line_to_remove )
        {
          c += *itTarget;
        }
      }

      foreach ( const unsigned& target, c )
      {
        if ( target == line_to_remove )
        {
          g.remove_target( target );
          g.add_target( line_to_use );
        }
        else
        {
          g.remove_target( target );
          g.add_target( target - 1 );
        }
      }
    }

    /* meta data */
    std::vector<std::string> inputs = circ.inputs();
    std::vector<std::string> outputs = circ.outputs();
    std::vector<constant> constants = circ.constants();
    std::vector<bool> garbage = circ.garbage();

    inputs.erase( inputs.begin() + line_to_remove );
    constants.erase( constants.begin() + line_to_remove );

    outputs.at( line_to_use ) = outputs.at( line_to_remove );
    garbage.at( line_to_use ) = garbage.at( line_to_remove );

    outputs.erase( outputs.begin() + line_to_remove );
    garbage.erase( garbage.begin() + line_to_remove );

    circ.set_lines( circ.lines() - 1 );

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );
  }

  bool line_reduction( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    /* settings */
    unsigned max_window_lines              = get<unsigned>( settings, "max_window_lines", 6u );
    unsigned max_grow_up_window_lines      = get<unsigned>( settings, "max_grow_up_window_lines", 9u );
    unsigned window_variables_threshold    = get<unsigned>( settings, "window_variables_threshold", 17u );
    simulation_func simulation             = get<simulation_func>( settings, "simulation", simple_simulation_func() );
    window_synthesis_func window_synthesis = get<window_synthesis_func>( settings, "window_synthesis", embed_and_synthesize() );

    /* statistics */
    unsigned num_considered_windows   = 0u;
    unsigned skipped_max_window_lines = 0u;
    unsigned skipped_ambiguous_line   = 0u;
    unsigned skipped_no_constant_line = 0u;
    unsigned skipped_synthesis_failed = 0u;

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    copy_circuit( base, circ );

    std::vector<unsigned> original_lines( base.lines() );
    std::copy( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( base.lines() ), original_lines.begin() );

    std::vector<unsigned> lines_to_skip;
    unsigned max_lines = max_window_lines;

    while ( true )
    {
      unsigned last_control_position;
      unsigned garbage_line = find_best_garbage_line( circ, lines_to_skip, original_lines, last_control_position );

      if ( garbage_line == circ.lines() )
      {
        break;
      }

      if ( statistics )
      {
        ++num_considered_windows;
      }

      circuit window = find_window_with_max_lines( circ, last_control_position, max_lines );

      /* find constant line */
      unsigned constant_line = find_constant_line( circ, window.offset() + window.num_gates() );
      if ( constant_line == circ.lines() )
      {
        if ( statistics )
        {
          ++skipped_no_constant_line;
        }

        lines_to_skip += original_lines.at( garbage_line );
        max_lines = max_window_lines;

        continue;
      }

      /* index_map */
      unsigned orig_lines;
      std::vector<unsigned> index_map;
      boost::tie( orig_lines, index_map ) = window.filter();

      std::vector<unsigned long long> assignments;

      if ( window.offset() == 0u ) // easy case: window starts on the left side
      {
        circuit zero = subcircuit( circ, 0u, 0u, index_map );

        // non constant inputs
        unsigned non_constant_lines = 0u;
        std::vector<constant> zero_constants;
        for ( std::vector<unsigned>::const_iterator itIndexMap = index_map.begin(); itIndexMap != index_map.end(); ++itIndexMap )
        {
          zero_constants += circ.constants().at( *itIndexMap );
          if ( !circ.constants().at( *itIndexMap ) )
          {
            ++non_constant_lines;
          }
        }
        circuit zero_copy( zero.lines() );
        append_circuit( zero_copy, zero );
        zero_copy.set_constants( zero_constants );

        properties::ptr ps_settings( new properties() );
        ps_settings->set( "keep_full_output", true );

        for ( unsigned long long input = 0ull; input < ( 1ull << non_constant_lines ); ++input )
        {
          boost::dynamic_bitset<> input_vec( non_constant_lines, input ), output_vec;
          partial_simulation( output_vec, zero_copy, input_vec, ps_settings );

          assignments += output_vec.to_ulong();
        }
      }
      else
      {
        std::vector<unsigned> before_filter;
        find_non_empty_lines( circ.begin(), circ.begin() + window.offset() + window.num_gates(), std::back_inserter( before_filter ) );
        std::sort( before_filter.begin(), before_filter.end() );
        before_filter.resize( std::unique( before_filter.begin(), before_filter.end() ) - before_filter.begin() );

        /* determine the number of window variables (no constants) */
        std::vector<constant> before_window_constants;
        foreach ( const unsigned& line, before_filter )
        {
          before_window_constants.push_back( circ.constants().at( line ) );
        }
        unsigned window_vars = std::count( before_window_constants.begin(), before_window_constants.end(), constant() );

        if ( window_vars >= window_variables_threshold )
        {
          if ( statistics )
          {
            ++skipped_max_window_lines;
          }

          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
          continue;
        }

        /* in this case the window starts in the beginning and we need the constant inputs */
        circuit before_window_sub = subcircuit( circ, 0u, window.offset(), before_filter );
        circuit before_window( before_window_sub.lines() );
        append_circuit( before_window, before_window_sub );
        before_window.set_constants( before_window_constants );
        before_window.set_garbage( std::vector<bool>( before_window.lines(), false ) );

        if ( window.lines() > 6 || window_vars < 12 )
        {
          properties::ptr ps_settings( new properties() );
          ps_settings->set( "keep_full_output", true );

          for ( unsigned long long input = 0ull; input < ( 1ull << window_vars ); ++input )
          {
            boost::dynamic_bitset<> input_vec( window_vars, input );
            boost::dynamic_bitset<> output;

            partial_simulation( output, before_window, input_vec, ps_settings );

            unsigned long long new_output = 0ull;

            // go through each line in window
            for ( unsigned long pos = 0; pos < index_map.size(); ++pos )
            {
              // line at pos
              unsigned line_index = index_map.at( pos );

              // this line relative in before_window
              unsigned before_window_line_index = std::find( before_filter.begin(), before_filter.end(), line_index ) - before_filter.begin();

              unsigned bit_at_before_window_line_index = output.test( before_window_line_index );

              new_output |= ( bit_at_before_window_line_index << pos );
            }

            assignments += new_output;
          }
        }
        else
        {
          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
          continue;
        }
      }

      std::vector<short> ov;
      std::vector<unsigned> order;
      garbage_to_ov( circ, window, index_map, garbage_line, ov, *circ.constants().at( constant_line ) );
      ov_to_order_vector( ov, order );

      /* create specification */
      binary_truth_table window_spec;
      if ( !create_window_specification( window, window_spec, assignments, ov, simulation ) )
      {
        if ( max_lines < max_grow_up_window_lines )
        {
          ++max_lines;
        }
        else
        {
          if ( statistics )
          {
            ++skipped_ambiguous_line;
          }

          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
        }
        continue;
      }

      circuit new_window, new_window_expanded;

      if ( !window_synthesis( new_window, window_spec, order ) )
      {
        if ( statistics )
        {
          ++skipped_synthesis_failed;
        }
        lines_to_skip += original_lines.at( garbage_line );
        max_lines = max_window_lines;
        continue;
      }

      expand_circuit( new_window, new_window_expanded, circ.lines(), index_map );

      // remember since we change the circuit
      circuit window_copy( window.lines() );
      append_circuit( window_copy, window );
      unsigned window_length = window.num_gates();
      unsigned window_offset = window.offset();

      for ( unsigned i = 0u; i < window_length; ++i )
      {
        circ.remove_gate_at( window_offset );
      }
      insert_circuit( circ, window_offset, new_window_expanded );
      remove_line( circ, constant_line, garbage_line );
    }

    if ( statistics )
    {
      statistics->set( "num_considered_windows", num_considered_windows );
      statistics->set( "skipped_max_window_lines", skipped_max_window_lines );
      statistics->set( "skipped_ambiguous_line", skipped_ambiguous_line );
      statistics->set( "skipped_no_constant_line", skipped_no_constant_line );
      statistics->set( "skipped_synthesis_failed", skipped_synthesis_failed );
    }

    return true;
  }

  optimization_func line_reduction_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = boost::bind( line_reduction, _1, _2, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}


