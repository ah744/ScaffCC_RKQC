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
 * @file swop.hpp
 *
 * @brief SWOP - Synthesis With Output Permutation
 */

#ifndef SWOP_HPP
#define SWOP_HPP

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/scoped_ptr.hpp>

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>
#include <core/functions/clear_circuit.hpp>
#include <core/functions/copy_circuit.hpp>
#include <core/utils/costs.hpp>
#include <core/utils/timer.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Functor which can be called after each SWOP iteration
   *
   * This functor is used for the \em stepfunc setting in the
   * \ref revkit::swop "swop" synthesis algorithm. It takes no arguments
   * and returns no value.
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<void()> swop_step_func;

  /**
   * @brief SWOP Synthesis Approach
   *
   * This is an implementation of the SWOP (Synthesis with Output Permutation) synthesis approach as introduced in [\ref WGDD09].
   * Thereby it is generic and can be used with every truth table based synthesis approach, which gets a circuit and
   * a truth table as parameters.
   *
   * @section sec_example_swop Example
   * This example used Boost.Bind to make a fitting operator from the transformation_based_synthesis function, taking
   * two parameters, the circuit and the truth table.
   * @code
   * #include <boost/bind.hpp>
   *
   * properties::ptr tbs_settings( new properties() );
   * // initialize settings for transformation based synthesis
   * properties::ptr tbs_statistics( new properties() );
   *
   * circuit circ;
   * binary_truth_table spec = // obtained from somewhere
   *
   * properties::ptr settings( new properties() ); // swop settings
   * settings->set( "synthesis", transformation_based_synthesis_func( settings, statistics ) );
   * swop( circ, spec, settings );
   * @endcode
   *
   * @param circ Circuit, must be empty, is filled with gate by the algorithm
   * @param spec Truth table used for synthesis
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">enable</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">true</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">This setting enables the SWOP algorithm to do swapping of the outputs at all. This is useful, when using SWOP as an additional option to a synthesis approach, only one call is necessary, i.e. using swop with the respective synthesis approach but disabling the swapping. Then the algorithm behaves as called stand-alone.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">exhaustive</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">false</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">When set to true, all possible permutations are checked, otherwise sifting is used to find a permutation, which may not be optimal.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">synthesis</td>
   *     <td class="indexvalue">\ref revkit::truth_table_synthesis_func "truth_table_synthesis_func"</td>
   *     <td class="indexvalue">\ref revkit::transformation_based_synthesis_func "transformation_based_synthesis_func()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Synthesis function to be used with the SWOP algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">cost_function</td>
   *     <td class="indexvalue">\ref revkit::cost_function "cost_function"</td>
   *     <td class="indexvalue">\ref revkit::gate_costs "costs_by_circuit_func( gate_costs() )"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Cost function to determine which circuit to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">stepfunc</td>
   *     <td class="indexvalue">\ref revkit::swop_step_func "swop_step_func"</td>
   *     <td class="indexvalue">\ref revkit::swop_step_func "swop_step_func()" <i>Empty functor</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">This functor is called after each iteration.</td>
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
   * @since 1.0
   */
  bool swop( circuit& circ, const binary_truth_table& spec,
             properties::ptr settings = properties::ptr(),
             properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::swop "swop" algorithm
   *
   * @param settings Settings (see \ref revkit::swop "swop")
   * @param statistics Statistics (see \ref revkit::swop "swop")
   *
   * @return A functor which complies with the \ref revkit::truth_table_synthesis_func "truth_table_synthesis_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  truth_table_synthesis_func swop_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* SWOP_HPP */
