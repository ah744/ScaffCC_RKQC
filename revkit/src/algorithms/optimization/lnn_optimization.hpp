/**
 * @file lnn_optimization.hpp
 *
 * @brief Linear nearest Neighbor
 */

#ifndef LNN_OPTIMIZATION_HPP
#define LNN_OPTIMIZATION_HPP

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/optimization/optimization.hpp>

namespace revkit
{

/**
 * @brief Linear nearest Neighbor
 *
 * Algorith implements a linear nearest neighbor approach
 *
 */
bool lnn_optimization( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

/**
 * @brief Functor for the lnn_optimization algorithm
 *
 * @param settings Settings (see lnn_optimization)
 * @param statistics Statistics (see lnn_optimization)
 *
 * @return A functor which complies with the optimization_func interface
 *
 */
optimization_func lnn_optimization_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* LNN_OPTIMIZATION_HPP */