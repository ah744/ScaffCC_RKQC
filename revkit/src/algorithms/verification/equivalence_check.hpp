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
 * @file equivalence_check.hpp
 *
 * @brief SAT-based equivalence check (respects garbage outputs and constant inputs)
 */

#ifndef EQUIVALENCE_CHECK_HPP
#define EQUIVALENCE_CHECK_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>
 
#include <algorithms/verification/verification.hpp>

namespace revkit
{
  /**
   * @brief Functional and formal equivalence check. 
   *
   * This function implements the SAT-based equivalence checker as introduced in [\ref WGMD09]. 
   *
   * @param spec       Circuit (Specification)
   * @param impl       Circuit to be verified (Implementation)
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">solver</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">"MiniSAT"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The solver to be used in the approach.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">max_counterexample</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">10u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The number of maximal generated counterexample.</td>
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
   *   <tr>
   *     <td class="indexvalue">counterexample</td>
   *     <td class="indexvalue">counterexample</td>
   *     <td class="indexvalue">Generated counterexample if the specification and the implementation differ.</td>
   *   </tr>
   * </table>
   *
   * @return true if successful, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */

  bool equivalence_check (  
      const circuit& spec
      , const circuit& impl
      , properties::ptr settings = properties::ptr ()
      , properties::ptr statistics = properties::ptr () ); 


  /** 
   * @brief Functor for the \ref revkit::equivalence_check "equivalence_check" algorithm
   *
   * @param settings Settings (see \ref revkit::equivalence_check "equivalence_check")
   * @param statistics Statistics (see \ref revkit::equivalence_check "equivalence_check")
   *
   * @return A functor which complies with the \ref revkit::equivalence_check "equivalence_check_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  equivalence_func equivalence_check_func (properties::ptr settings = properties::ptr ( new properties ()), properties::ptr statistics = properties::ptr ( new properties ()) );

}

#endif 
