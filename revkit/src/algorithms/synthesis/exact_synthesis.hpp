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
 * @file exact_synthesis.hpp
 *
 * @brief Exact Synthesis of Reversible Networks
 */

#ifndef EXACT_SYNTHESIS_HPP
#define EXACT_SYNTHESIS_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Synthesizes a minimal circuit (with respect to the number of gates) using the SAT-based exact synthesis approach as presented in [\ref GWDD09].
   *
   *
   * @param circ       Empty Circuit
   * @param spec       Function Specification (has to be fully specified)
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
   *     <td rowspan="2" class="indexvalue">spec_incremental</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">true</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Specifies, whether the incremental encoding should be used.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">max_depth</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">20u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The maximal considered circuit depth.</td>
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
   *
   * @return true if successful, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool exact_synthesis( circuit& circ
       , const binary_truth_table& spec
       , properties::ptr settings = properties::ptr ()
       , properties::ptr statistics = properties::ptr () );

  /**
   * @brief Functor for the \ref revkit::exact_synthesis "exact_synthesis" algorithm
   *
   * @param settings Settings (see \ref revkit::exact_synthesis "exact_synthesis")
   * @param statistics Statistics (see \ref revkit::exact_synthesis "exact_synthesis")
   *
   * @return A functor which complies with the \ref revkit::exact_synthesis "exact_synthesis_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  truth_table_synthesis_func exact_synthesis_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* EXACT_SYNTHESIS_HPP */
