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

#include <core/circuit.hpp>

#include "vector_py.hpp"
#include "py_optional.hpp"

using namespace boost::python;
using namespace revkit;

void export_core();
void export_core_functions();
void export_core_io();
void export_core_utils();
void export_algorithms();
void export_unstable();

struct transformation_based_synthesis1_fwd
{
  template<class R, class A0, class A1, class A2>
  R operator()( boost::type<R>, A0 const& a0, A1 const& a1, A2 const& a2 )
  {
    return transformation_based_func1( a0, a1, a2 );
  }
};

BOOST_PYTHON_MODULE( revkit_python )
{
  python_vector<std::string>( "std_vector_string" );
  python_vector<bool>( "std_vector_bool" );
  python_vector<constant>( "std_vector_constant" );

  python_optional<bool>();

  export_core();
  export_core_functions();
  export_core_io();
  export_core_utils();
  export_algorithms();
  export_unstable();
}
