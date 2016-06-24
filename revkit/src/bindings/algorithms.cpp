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

#include <boost/python.hpp>
#include <boost/python/args.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/format.hpp>

#include <algorithms/optimization/adding_lines.hpp>
#include <algorithms/optimization/line_reduction.hpp>
#include <algorithms/optimization/lnn_optimization.hpp>
#include <algorithms/optimization/window_optimization.hpp>

#include <algorithms/synthesis/bdd_synthesis.hpp>
#include <algorithms/synthesis/embed_truth_table.hpp>
#include <algorithms/synthesis/esop_synthesis.hpp>
#include <algorithms/synthesis/exact_synthesis.hpp>
#include <algorithms/synthesis/kfdd_synthesis.hpp>
#include <algorithms/synthesis/quantum_decomposition.hpp>
#include <algorithms/synthesis/reed_muller_synthesis.hpp>
#include <algorithms/synthesis/swop.hpp>
#include <algorithms/synthesis/synthesis.hpp>
#include <algorithms/synthesis/transformation_based_synthesis.hpp>
#include <algorithms/synthesis/transposition_based_synthesis.hpp>

#include <algorithms/simulation/partial_simulation.hpp>
#include <algorithms/simulation/sequential_simulation.hpp>
#include <algorithms/simulation/simple_simulation.hpp>

#include <algorithms/verification/equivalence_check.hpp>

#include <cudd.h>

#include "py_boost_function.hpp"

using namespace boost::python;
using namespace revkit;

// Algorithms / Optimization
select_window_func shift_window_selection_func( unsigned window_length, unsigned offset )
{
  shift_window_selection sws;
  sws.window_length = window_length;
  sws.offset = offset;
  return sws;
}

select_window_func line_window_selection_func()
{
  line_window_selection lws;
  return lws;
}

optimization_func resynthesis_optimization_func( truth_table_synthesis_func synthesis, simulation_func simulation )
{
  resynthesis_optimization ro;
  ro.synthesis = synthesis;
  ro.simulation = simulation;
  return ro;
}

window_synthesis_func embed_and_synthesize1( embedding_func embedding, truth_table_synthesis_func synthesis, unsigned timeout )
{
  embed_and_synthesize eas;
  eas.embedding = embedding;
  eas.synthesis = synthesis;
  eas.timeout = timeout;
  return eas;
}

// Algorithms / Optimization
BOOST_PYTHON_FUNCTION_OVERLOADS( adding_lines_overloads, adding_lines, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( adding_lines_func_overloads, adding_lines_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( line_reduction_overloads, line_reduction, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( line_reduction_func_overloads, line_reduction_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( lnn_optimization_overloads, lnn_optimization, 2, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS( lnn_optimization_func_overloads, lnn_optimization_func, 0 ,2)
BOOST_PYTHON_FUNCTION_OVERLOADS( window_optimization_overloads, window_optimization, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( window_optimization_func_overloads, window_optimization_func, 0, 2 )

// Algorithms / Simulation
BOOST_PYTHON_FUNCTION_OVERLOADS( simple_simulation_overloads, simple_simulation, 3, 5 )
BOOST_PYTHON_FUNCTION_OVERLOADS( simple_simulation_func_overloads, simple_simulation_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( partial_simulation_overloads, partial_simulation, 3, 5 )
BOOST_PYTHON_FUNCTION_OVERLOADS( partial_simulation_func_overloads, partial_simulation_func, 0, 2 )

// Algorithms / Synthesis
BOOST_PYTHON_FUNCTION_OVERLOADS( bdd_synthesis_overloads, bdd_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( bdd_synthesis_func_overloads, bdd_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( embed_truth_table_overloads, embed_truth_table, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( embed_truth_table_func_overloads, embed_truth_table_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( esop_synthesis_overloads, esop_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( esop_synthesis_func_overloads, esop_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( exact_synthesis_overloads, exact_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( exact_synthesis_func_overloads, exact_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( kfdd_synthesis_overloads, kfdd_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( kfdd_synthesis_func_overloads, kfdd_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( equivalence_check_overloads, equivalence_check, 2, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS( quantum_decomposition_overloads, quantum_decomposition, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( quantum_decomposition_func_overloads, quantum_decomposition_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( reed_muller_synthesis_overloads, reed_muller_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( reed_muller_synthesis_func_overloads, reed_muller_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( swop_overloads, swop, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( swop_func_overloads, swop_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( transformation_based_synthesis_overloads, transformation_based_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( transformation_based_synthesis_func_overloads, transformation_based_synthesis_func, 0, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( transposition_based_synthesis_overloads, transposition_based_synthesis, 2, 4 )
BOOST_PYTHON_FUNCTION_OVERLOADS( transposition_based_synthesis_func_overloads, transposition_based_synthesis_func, 0, 2 )

cube_reordering_func no_reordering1()
{
  return &no_reordering;
}

cube_reordering_func weighted_reordering1( float alpha, float beta )
{
  return weighted_reordering( alpha, beta );
}

gate_decomposition_func standard_decomposition1()
{
  return standard_decomposition();
}

/* std::map<K, V> to dict */
template<typename K, typename V>
struct map_to_python
{
  static PyObject* convert( const std::map<K, V>& value )
  {
    dict d;

    for ( typename std::map<K, V>::const_iterator it = value.begin(); it != value.end(); ++it )
    {
      d[it->first] = it->second;
    }

    return incref( d.ptr() );
  }
};

bool sequential_simulation1( list& loutputs, const circuit& circ,
                             const list& linputs,
                             properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() )
{
  stl_input_iterator<boost::dynamic_bitset<> > begin( linputs ), end;
  std::vector<boost::dynamic_bitset<> > inputs;
  std::copy( begin, end, std::back_inserter( inputs ) );

  std::vector<boost::dynamic_bitset<> > outputs;
  bool r = revkit::sequential_simulation( outputs, circ, inputs, settings, statistics );

  std::for_each( outputs.begin(), outputs.end(), boost::bind( &list::append<boost::dynamic_bitset<> >, &loutputs, _1 ) );

  return r;
}

BOOST_PYTHON_FUNCTION_OVERLOADS( sequential_simulation1_overloads, sequential_simulation1, 3, 5 )

void export_algorithms()
{

  // Optimization
  def_functor<bool(circuit&, const circuit&)>( "optimization_func", "" );

  def_function<bool(circuit&, binary_truth_table&, const std::vector<unsigned>&)>( "window_synthesis_func", "" );
  def_function<circuit(const circuit&)>( "select_window_func", "" );

  def( "py_adding_lines", adding_lines, adding_lines_overloads() );
  def( "py_adding_lines_func", adding_lines_func, adding_lines_func_overloads() );
  def( "py_line_reduction", line_reduction, line_reduction_overloads() );
  def( "py_line_reduction_func", line_reduction_func, line_reduction_func_overloads() );
def( "py_lnn_optimization", lnn_optimization, lnn_optimization_overloads() );
  def( "py_lnn_optimization_func", lnn_optimization_func, lnn_optimization_func_overloads() );
  def( "py_window_optimization", window_optimization, window_optimization_overloads() );
  def( "py_window_optimization_func", window_optimization_func, window_optimization_func_overloads() );

  def( "py_embed_and_synthesize", embed_and_synthesize1 );
  def( "py_shift_window_selection_func", shift_window_selection_func );
  def( "py_line_window_selection_func", line_window_selection_func );
  def( "py_resynthesis_optimization_func", resynthesis_optimization_func );

  // Simulation
  def_functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)>( "simulation_func", "" );
  def_functor<bool(std::vector<boost::dynamic_bitset<> >&, const circuit&, const std::vector<boost::dynamic_bitset<> >&)>( "multi_step_simulation_func", "" );

  def_function<void(const gate&, const boost::dynamic_bitset<>&)>( "step_result_func", "" );
def_function<boost::dynamic_bitset<>(const std::map<std::string, boost::dynamic_bitset<> >&, const boost::dynamic_bitset<>&)>( "sequential_step_result_func", "" );
  boost::python::to_python_converter<std::map<std::string, boost::dynamic_bitset<> >, map_to_python<std::string, boost::dynamic_bitset<> > >();

  def( "py_simple_simulation", static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, properties::ptr, properties::ptr)>( simple_simulation ), simple_simulation_overloads() );
  def( "py_simple_simulation_func", simple_simulation_func, simple_simulation_func_overloads() );
  def( "py_partial_simulation", static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, properties::ptr, properties::ptr)>( partial_simulation ), partial_simulation_overloads() );
  def( "py_partial_simulation_func", partial_simulation_func, partial_simulation_func_overloads() );

  // Synthesis
  def_functor<bool(circuit&, const binary_truth_table&)>( "truth_table_synthesis_func", "" );
  def_functor<bool(circuit&, const std::string&)>( "pla_blif_synthesis_func", "" );
  def_functor<bool(binary_truth_table&, const binary_truth_table&)>( "embedding_func", "" );
  //  def_functor<bool(circuit&, const circuit&)>( "decomposition_func", "" ); NOTE

  def_function<void(circuit&, const gate&)>( "gate_decomposition_func", "" );
  def_function<void()>( "swop_step_func", "" );
  def_function<void(std::vector<std::pair<binary_truth_table::cube_type, binary_truth_table::cube_type> >&)>( "cube_reordering_func", "" );

  def( "py_bdd_synthesis", bdd_synthesis, bdd_synthesis_overloads() );
  def( "py_bdd_synthesis_func", bdd_synthesis_func, bdd_synthesis_func_overloads() );
  def( "py_embed_truth_table", embed_truth_table, embed_truth_table_overloads() );
  def( "py_embed_truth_table_func", embed_truth_table_func, embed_truth_table_func_overloads() );
  def( "py_esop_synthesis", esop_synthesis, esop_synthesis_overloads() );
  def( "py_esop_synthesis_func", esop_synthesis_func, esop_synthesis_func_overloads() );
  def( "py_exact_synthesis", exact_synthesis, exact_synthesis_overloads() );
  def( "py_exact_synthesis_func", exact_synthesis_func, exact_synthesis_func_overloads() );
  def( "py_kfdd_synthesis", kfdd_synthesis, kfdd_synthesis_overloads() );
  def( "py_kfdd_synthesis_func", kfdd_synthesis_func, kfdd_synthesis_func_overloads() );
  def( "py_equivalence_check", equivalence_check, equivalence_check_overloads () );
  def( "py_quantum_decomposition", quantum_decomposition, quantum_decomposition_overloads() );
  def( "py_quantum_decomposition_func", quantum_decomposition_func, quantum_decomposition_func_overloads() );
  def( "py_reed_muller_synthesis", reed_muller_synthesis, reed_muller_synthesis_overloads() );
  def( "py_reed_muller_synthesis_func", reed_muller_synthesis_func, reed_muller_synthesis_func_overloads() );
  def( "py_swop", swop, swop_overloads() );
  def( "py_swop_func", swop_func, swop_func_overloads() );
  def( "py_transformation_based_synthesis", transformation_based_synthesis, transformation_based_synthesis_overloads() );
  def( "py_transformation_based_synthesis_func", transformation_based_synthesis_func, transformation_based_synthesis_func_overloads() );
  def( "py_transposition_based_synthesis", transposition_based_synthesis, transposition_based_synthesis_overloads() );
  def( "py_transposition_based_synthesis_func", transposition_based_synthesis_func, transposition_based_synthesis_func_overloads() );

  def( "no_reordering", no_reordering1 );
  def( "py_weighted_reordering", weighted_reordering1 );
  def( "standard_decomposition", standard_decomposition1 );

  def( "py_sequential_simulation", sequential_simulation1, sequential_simulation1_overloads() );

}
