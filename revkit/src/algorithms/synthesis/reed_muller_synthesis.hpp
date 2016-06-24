/**
 * @file reed_muller_synthesis.hpp
 *
 * @brief Synthesis algorithm based on Reed Muller Spectra
 */

#ifndef REED_MULLER_SYNTHESIS_HPP
#define REED_MULLER_SYNTHESIS_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Synthesis algorithm based on Reed Muller Spectra
   *
   * This function implements the algorithm published in [\ref MDM07].
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
   *     <td rowspan="2" class="indexvalue">bidirectional</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">true</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Use the bidirectional approach as described in [\ref MDM07].</td>
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
   * @since  1.3
   */
  bool reed_muller_synthesis( circuit& circ, const binary_truth_table& spec, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the reed_muller_synthesis algorithm
   *
   * @param settings Settings (see reed_muller_synthesis)
   * @param statistics Statistics (see reed_muller_synthesis)
   *
   * @return A functor which complies with the truth_table_synthesis_func interface
   *
   * @author RevKit
   * @since  1.3
   */
  truth_table_synthesis_func reed_muller_synthesis_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* REED_MULLER_SYNTHESIS_HPP */
