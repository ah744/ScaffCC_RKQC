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
 * @file esop_synthesis.hpp
 *
 * @brief ESOP Based Synthesis
 */

#ifndef ESOP_SYNTHESIS_HPP
#define ESOP_SYNTHESIS_HPP

#include <vector>

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit 
{

  /**
   * @brief Functor for cubes reordering in ESOP based synthesis
   *
   * This functor reorders the cubes in place and gets as single parameter
   * a mutable vector of pairs, which contains input (first) and output cubes (second).
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<void(std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >&)> cube_reordering_func;

  /**
   * @brief Empty functor for \ref revkit::cube_reordering_func "cube_reordering_func"
   *
   * This functor is empty and does not sort the cubes, so the order remains the same.
   *
   * @author RevKit
   * @since  1.0
   */
  void no_reordering( std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >& cubes );

  /**
   * @brief Cubes reordering strategy as proposed in [\ref FTR07]
   *
   * This functor implements the reordering strategy as proposed in [\ref FTR07].
   * In that strategy two variables \p alpha and \p beta are used to
   * control the weight of variables by their appearance and their polarity, respectively.
   *
   * @author RevKit
   * @since  1.0
   */
  struct weighted_reordering
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    weighted_reordering();

    /**
     * @brief Constructor for adjusting the parameters
     *
     * In this parameters the values for \p alpha and \p beta can
     * be set directly.
     *
     * @param alpha Control variable for the weight of the variable frequency term
     * @param beta Control variable for the weight of the balanced variable term
     *
     * @author RevKit
     * @since  1.0
     */
    weighted_reordering( float alpha, float beta );
    
     /**
     * @brief Control variable for the weight of the variable frequency term
     *
     * Default value is \p 0.5
     *
     * @author RevKit
     * @since  1.0
     */
    float alpha;

     /**
     * @brief Control variable for the weight of the balanced variable term
     *
     * Default value is \p 0.5
     *
     * @author RevKit
     * @since  1.0
     */
    float beta;

    /**
     * @brief Functor operator implementation
     *
     * @param cubes Cubes to be reordered
     *
     * @author RevKit
     * @since  1.0
     */
    void operator()( std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >& cubes ) const;

  private:
    void reorder( std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >::iterator begin, std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >::iterator end, const std::vector<unsigned>& vars ) const;
  };

  /**
   * @brief ESOP Based Synthesis
   *
   * This algorithm implements the ESOP based synthesis approach as introduced in [\ref FTR07].
   * The basic approach, where each input signal requires to line for its positive and
   * negative polarity version, can be enabled by setting the property \em separate_polarities to \em true.
   * If one line is used for both polarities, which is the default case, a functor can be specified
   * to reorder the cubes in order to minimize inverter gates. Two functors are provided which are,
   * \ref revkit::no_reordering "no_reordering" which keeps the initial order from the truth table,
   * and \ref revkit::weighted_reordering "weighted_reordering" which is proposed in [\ref FTR07] as
   * reordering strategy.
   *
   * @param circ Circuit to be generated.
   * @param filename A file-name of a function as ESOP PLA representation.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">separate_polarities</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">false</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If \em true, the basic approach using two circuit lines for each input variable is used. Further, in that case, no reordering functor has to be specified.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">reordering</td>
   *     <td class="indexvalue">\ref revkit::cube_reordering_func "cube_reordering_func"</td>
   *     <td class="indexvalue">\ref revkit::weighted_reordering "weighted_reordering()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Function for reordering the cubes to obtain a better result by using less NOT gates.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">garbage_name</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">"g"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Name of the garbage outputs which are added by the algorithm for each input line.</td>
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
  bool esop_synthesis( circuit& circ, const std::string& filename, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::esop_synthesis "esop_synthesis" algorithm
   *
   * @param settings Settings (see \ref revkit::esop_synthesis "esop_synthesis")
   * @param statistics Statistics (see \ref revkit::esop_synthesis "esop_synthesis")
   *
   * @return A functor which complies with the \ref revkit::pla_blif_synthesis_func "pla_blif_synthesis_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  pla_blif_synthesis_func esop_synthesis_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* ESOP_SYNTHESIS_HPP */
