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
 * @file quantum_decomposition.hpp
 *
 * @brief Quantum Decomposition of Reversible Circuits
 */

#ifndef QUANTUM_DECOMPOSITION_HPP
#define QUANTUM_DECOMPOSITION_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Functor for the gate-wise decomposition
   *
   * The \ref revkit::quantum_decomposition "quantum_decomposition" algorithm has a setting to specify
   * the gate-wise decomposition. Per default, this is
   * \ref revkit::standard_decomposition "standard_decomposition".
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<void(circuit&, const gate&)> gate_decomposition_func;

  /**
   * @brief Default gate-wise decomposition
   *
   * This struct can be used as \ref revkit::gate_decomposition_func "gate_decomposition_func" to assign
   * to the \em gate_decomposition setting of the \ref revkit::quantum_decomposition "quantum_decomposition"
   * algorithm.
   *
   * @author RevKit
   * @since  1.0
   */
  struct standard_decomposition
  {
    /** 
     * @brief Functor Operator
     * 
     * Decomposes \p g and appends it to \p circ
     *
     * @param circ Circuit to be created during decomposition process
     * @param g Gate to be decomposed
     *
     * @author RevKit
     * @since  1.0
     */
    void operator()( circuit& circ, const gate& g ) const;
  };

  /**
   * @brief Quantum Decomposition of Reversible Circuits
   *
   * This algorithm decomposes a reversible circuit into a quantum circuit based on the work of [\ref BBC95 "BBC+95"] and [\ref MD03].
   * The resulting circuits do not necessarily coincide with the quantum costs calculated by \ref revkit::quantum_costs "quantum_costs", since
   * some further optimizations are not considered yet.
   *
   * @param circ Quantum circuit to be created
   * @param base Reversible circuit
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">helper_line_input</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">"w"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Name of the input of the helper line, if added by the algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">helper_line_output</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">"w"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Name of the output of the helper line, if added by the algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_decomposition</td>
   *     <td class="indexvalue">\ref revkit::decomposition_func "decomposition_func"</td>
   *     <td class="indexvalue">\ref revkit::standard_decomposition "standard_decomposition()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Gate-wise decomposition function to use.</td>
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
  bool quantum_decomposition( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::quantum_decomposition "quantum_decomposition" algorithm
   *
   * @param settings Settings (see \ref revkit::quantum_decomposition "quantum_decomposition")
   * @param statistics Statistics (see \ref revkit::quantum_decomposition "quantum_decomposition")
   *
   * @return A functor which complies with the \ref revkit::decomposition_func "decomposition_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  decomposition_func quantum_decomposition_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* QUANTUM_DECOMPOSITION_HPP */
