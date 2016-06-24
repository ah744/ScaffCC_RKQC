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
 * @file kfdd_synthesis.hpp
 *
 * @brief KFDD Based Synthesis
 */

#ifndef KFDD_SYNTHESIS_HPP
#define KFDD_SYNTHESIS_HPP

#include <string>

#include <core/circuit.hpp>
#include <core/properties.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  // NOTE define naming conventions for public enumerations
  /**
   * @brief Flags for default KFDD decomposition in kfdd_synthesis_settings
   *
   * The default decomposition type is especially important when no 
   * DTL reordering strategy is used.
   *
   * @author RevKit
   * @since  1.0
   */
  enum {
    /**
     * @brief Use Shannon as default
     */
    kfdd_synthesis_dtl_shannon,
    /**
     * @brief Use positive Davio as default
     */
    kfdd_synthesis_dtl_positive_davio,
    /**
     * @brief Use negative Davio as default
     */
    kfdd_synthesis_dtl_negative_davio };

  /**
   * @brief Flags for KFDD Reordering strategies in kfdd_synthesis_settings
   *
   * @author RevKit
   * @since  1.0
   */
  enum { 
    /**
     * @brief No reordering
     */
    kfdd_synthesis_reordering_none,
    /**
     * @brief Exact DTL and variable re-ordering according to an algorithm introduced by Friedman
     */
    kfdd_synthesis_reordering_exact_dtl_friedman,
    /**
     * @brief Exact DTL and variable re-ordering by permutation
     */
    kfdd_synthesis_reordering_exact_dtl_permutation,
    /**
     * @brief Heuristic DTL and variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_dtl_sifting,
    /**
     * @brief Exact variable re-ordering according to an algorithm introduced by Friedman
     */
    kfdd_synthesis_reordering_exact_friedman,
    /**
     * @brief Exact variable re-ordering by permutation
     */
    kfdd_synthesis_reordering_exact_permutation,
    /**
     * @brief Heuristic variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_sifting,
    /**
     * @brief Heuristic variable re-ordering by sifting followed by heuristic DTL and variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_sifting_and_dtl_sifting,
    /**
     * @brief Inversion of the variable ordering
     */
    kfdd_synthesis_reordering_inverse };

  /**
   * @brief Flags for the growth limit in kfdd_synthesis_settings
   */
  enum { 
    /**
     * @brief Relative growth limit: after each repositioning of a sifting variable the comparison size for the growing is actualized
     */
    kfdd_synthesis_growth_limit_relative = 'r',
    /**
     * @brief Absolute growth limit: the comparison size is the intial size of the DDs for the complete sifting-process
     */
    kfdd_synthesis_growth_limit_absolute = 'a' };

  /**
   * @brief Flags for the sifting method
   */
  enum {
    /**
     * @brief \b Random
     */
    kfdd_synthesis_sifting_method_random = 'r',
    /**
     * @brief \b Initial: The sifting variables are chosen in the order given before the sifting procedure starts
     */
    kfdd_synthesis_sifting_method_initial = 'i',
    /**
     * @brief \b Greatest: Chooses the variable in the level with the largest number of nodes
     */
    kfdd_synthesis_sifting_method_greatest = 'g',
    /**
     * @brief <b>Loser first:</b> Although the complete sifting process will reduce the number of DD-Nodes (or at least keep the same size if no improvement can be done) after each repositioning of a sifting variable there will occasionally be some levels that grow. The loser first strategy chooses the next sifting candidate as the variable in the level with the least increase in size
     */
    kfdd_synthesis_sifting_method_loser_first = 'l',
    /**
     * @brief \b Verify: Calculates the number of node eliminations due to the reduction rules of KFDDs if a variable is repositioned in a specific level. It then chooses the best position according to the highest count result
     */
    kfdd_synthesis_sifting_method_verify = 'v' };
         
  /**
   * @brief KFDD Based Synthesis
   *
   * This algorithm implements the KFDD based synthesis approach as introduced in [\ref SWD10]. Thereby, re-ordering strategies as well as different decomposition types can be used.
   *
   * @param circ Empty circuit, to be constructed by the algorithm
   * @param filename A functional representation as BLIF or PLA file
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">default_decomposition</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_dtl_shannon</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Default decomposition type when initially generating the KFDD.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">reordering</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_reordering_none</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Reordering of the KFDD.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sift_factor</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">2.5</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting factor, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sifting_growth_limit</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_growth_limit_absolute</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting growth limit, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sifting_method</td>
   *     <td class="indexvalue">char</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_sifting_method_verify</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting method, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">dotfilename</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">std::string()</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If not empty a DOT representation of the KFDD is dumped to the file-name.</td>
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
   *     <td class="indexvalue">node_count</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of nodes of the KFDD.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @author RevKit
   * @since  1.0
   */
  bool kfdd_synthesis( circuit& circ, const std::string& filename,
                       properties::ptr settings = properties::ptr(),
                       properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func" algorithm
   *
   * @param settings Settings (see \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func")
   * @param statistics Statistics (see \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func")
   *
   * @return A functor which complies with the \ref revkit::pla_blif_synthesis_func "pla_blif_synthesis_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  pla_blif_synthesis_func kfdd_synthesis_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* KFDD_SYNTHESIS_HPP */
