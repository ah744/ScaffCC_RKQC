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

/**
 * @file window_optimization.hpp
 *
 * @brief Window Optimization
 */

#ifndef WINDOW_OPTIMIZATION_HPP
#define WINDOW_OPTIMIZATION_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>

#include <algorithms/optimization/optimization.hpp>
#include <algorithms/simulation/simulation.hpp>
#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Functor for selecting the windows
   *
   * The selection of the windows can be defined by functors.
   * Two are already implemented, namely the \ref revkit::line_window_selection "Line Window Selection"
   * and the \ref revkit::shift_window_selection "Shift Window Selection".
   *
   * @section sec_example_window_optimization_settings1 Example: How to assign a window selection method
   *
   * @code
   * revkit::shift_window_selection sws;
   * sws.window_length = 5;
   * sws.offset = 4;
   * 
   * revkit::properties::ptr settings( new revkit::properties() );
   * settings->set( "select_window", sws );
   *
   * // or for line window selection without specifying a parameter
   *
   * settings->set( "select_window", revkit::line_window_selection() );
   * @endcode
   *
   * @section sec_example_window_optimization_settings2 Example: How to create a new window selection functor
   * 
   * New window selection functors can be easily defined by creating a struct with the \b operator() method
   * having the signature as follows in this example. In this example the first half of the circuit is chosen as a window.
   * In the implementation of window_optimization, this functor is called iteratively until an empty circuit is
   * returned. Thus, the \p done flag is set to \b true after the first call and the functor returns the empty
   * circuit in the second call.
   *
   * @code
   * struct whole_circuit_selection
   * {
   *   whole_circuit_selection() : done( false ) {}
   *
   *   revkit::circuit operator()( const revkit::circuit& base )
   *   {
   *     if ( done ) return revkit::circuit();
   *
   *     done = true;
   *     return revkit::subcircuit( base, 0u, base.num_gates() / 2 );
   *   }
   *
   * private:
   *   bool done;
   * };
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<circuit(const circuit&)> select_window_func;

  /**
   * @brief Window Selection functor based on Shift Window Selection
   *
   * This functor is based on the shift window selection method as introduced in [\ref SWDD10].
   *
   * @author RevKit
   * @since  1.0
   */
  struct shift_window_selection
  {
    /**
     * @brief Standard Constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    shift_window_selection();

    /**
     * @brief Length of the windows
     *
     * Default value is \b 10.
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned window_length;

    /**
     * @brief Offset determining for how many gates the window is shifted
     *
     * When setting offset = window_length, the original circuit is partitioned.
     *
     * Default value is \b 1.
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned offset;

    /**
     * @brief Operator to determine the current window.
     *
     * Returns an empty circuit when no more windows can be determined.
     *
     * @param base The original circuit
     *
     * @return A new window or the empty circuit
     *
     * @author RevKit
     * @since  1.0
     */
    circuit operator()( const circuit& base );

  private:
    /** @cond */
    unsigned pos;
    /** @endcond */
  };

  /**
   * @brief Window Selection functor based on Line Window Selection
   *
   * This functor is based on the line window selection method as introduced in [\ref SWDD10].
   *
   * @author RevKit
   * @since  1.0
   */
  struct line_window_selection
  {
    /**
     * @brief Standard Constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    line_window_selection();

    /**
     * @brief Operator to determine the current window.
     *
     * Returns an empty circuit when no more windows can be determined.
     *
     * @param base The original circuit
     *
     * @return A new window or the empty circuit
     *
     * @author RevKit
     * @since  1.0
     */
    circuit operator()( const circuit& base );

  private:
    /** @cond */
    unsigned num_lines;
    unsigned line_count;
    unsigned pos;
    /** @endcond */
  };

  /**
   * @brief Re-synthesis optimization (Wrapper for window_optimization)
   *
   * This functor is a wrapper to perform re-synthesis as a optimization function.
   * Therefore, the base circuit is simulated to get the truth table and this truth
   * table is synthesized using a synthesis algorithm given by the parameter \p synthesis.
   *
   * @author RevKit
   * @since  1.0
   */
  struct resynthesis_optimization
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    resynthesis_optimization();

    /**
     * @brief Synthesis method to re-synthesize the circuit
     *
     * Default value is \b revkit::transformation_based_synthesis_func
     *
     * @author RevKit
     * @since  1.0
     */
    truth_table_synthesis_func synthesis;

    /**
     * @brief Simulation method for creating the truth table
     *
     * Default value is \b revkit::simple_simulation_func
     * 
     * @author RevKit
     * @since  1.0
     */
    simulation_func simulation;

    /**
     * @brief Functor which wraps the re-synthesis algorithm as an optimization algorithm
     *
     * The signature matches the signature of revkit::optimization_func.
     *
     * @param new_window New window to be created
     * @param old_window Original window
     * @return true on success
     *
     * @author RevKit
     * @since  1.0
     */
    bool operator()( circuit& new_window, const circuit& old_window ) const;
  };

  /**
   * @brief Window Optimization
   *
   * This algorithm implements the window optimization approach as presented in [\ref SWDD10].
   * The implementation is very generic and depends heavily on the functors defined in \p settings.
   *
   * In a loop, a new window is selected using the \em select_window property,
   * and in case a window was found, the optimization approach using the \em optimization property is applied.
   *
   * The resulting new window is compared to the extracted one using the cost metric defined in the \em cost_function property.
   *
   * @param circ Optimized circuit to be generated
   * @param base Original circuit
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">select_window</td>
   *     <td class="indexvalue">\ref revkit::select_window_func "select_window_func"</td>
   *     <td class="indexvalue">\ref revkit::shift_window_selection "shift_window_selection()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Functor which is used to select the windows to optimize.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">optimization</td>
   *     <td class="indexvalue">\ref revkit::optimization_func "optimization_func"</td>
   *     <td class="indexvalue">\ref revkit::resynthesis_optimization "resynthesis_optimization()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Functor used to optimize the selected window.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">cost_function</td>
   *     <td class="indexvalue">\ref revkit::cost_function "cost_function"</td>
   *     <td class="indexvalue">\ref revkit::gate_costs "costs_by_circuit_func( gate_costs() )"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Cost function to determine whether the optimized circuit is cheaper.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @author RevKit
   * @since  1.0
   */
  bool window_optimization( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::window_optimization "window_optimization" algorithm
   *
   * @param settings Settings (see \ref revkit::window_optimization "window_optimization")
   * @param statistics Statistics (see \ref revkit::window_optimization "window_optimization")
   *
   * @return A functor which complies with the \ref revkit::optimization_func "optimization_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  optimization_func window_optimization_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* WINDOW_OPTIMIZATION_HPP */
