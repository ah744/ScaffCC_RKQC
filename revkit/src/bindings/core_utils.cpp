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
#include <boost/python/object.hpp>
#include <boost/python/stl_iterator.hpp>

#include <core/utils/costs.hpp>
#include <core/utils/program_options.hpp>

#include "py_boost_function.hpp"

using namespace boost::python;
using namespace revkit;

costs_by_circuit_func gate_costs1()
{
  return gate_costs();
}

costs_by_circuit_func line_costs1()
{
  return line_costs();
}

costs_by_gate_func quantum_costs1()
{
  return quantum_costs();
}

costs_by_gate_func transistor_costs1()
{
  return transistor_costs();
}

unsigned costs1( const circuit& circ, const costs_by_circuit_func& f )
{
  return costs( circ, f );
}

unsigned costs2( const circuit& circ, const costs_by_gate_func& f )
{
  return costs( circ, f );
}

template<typename T>
program_options& program_options_add_option1( program_options& opts, const char* name, const char* description )
{
  opts.add_options()( name, boost::program_options::value<T>(), description );
  return opts;
}

template<typename T>
program_options& program_options_add_option2( program_options& opts, const char* name, T default_value, const char* description )
{
  opts.add_options()( name, boost::program_options::value<T>()->default_value( default_value ), description );
  return opts;
}

struct program_options_costs_visitor : public boost::static_visitor<object>
{
  object operator()( const costs_by_circuit_func& f ) const
  {
    return object( f );
  }

  object operator()( const costs_by_gate_func& f ) const
  {
    return object( f );
  }
};

object program_options_costs( program_options& opts )
{
  cost_function cf = opts.costs();
  return boost::apply_visitor( program_options_costs_visitor(), cf );
}

void program_options_parse( program_options& opts, object o )
{
  stl_input_iterator<char*> begin( o ), begin2( o ), end;
  int argc = std::distance( begin, end );
  char ** argv = new char*[argc];
  std::copy( begin2, end, argv );
  opts.parse( argc, argv );
  delete[] argv;
}

object program_options_value( program_options& opts, const std::string& name )
{
  boost::any value = opts.variables_map()[name].value();
  const std::type_info& type = value.type();

  if ( type == typeid( std::string ) )
  {
    return object( boost::any_cast<std::string>( value ) );
  }
  else if ( type == typeid( unsigned ) )
  {
    return object( boost::any_cast<unsigned>( value ) );
  }
  else if ( type == typeid( double ) )
  {
    return object( boost::any_cast<double>( value ) );
  }
  else
  {
    return object( 0 );
  }
}

void export_core_utils()
{

  def_function<unsigned long long(const circuit&)>( "costs_by_circuit_func", "" );
  def_function<unsigned long long(const gate&, unsigned)>( "costs_by_gate_func", "" );

  def( "gate_costs", gate_costs1 );
  def( "line_costs", line_costs1 );
  def( "quantum_costs", quantum_costs1 );
  def( "transistor_costs", transistor_costs1 );

  def( "py_costs", costs1 );
  def( "py_costs", costs2 );

  class_<boost::program_options::options_description>( "options_description" )
    .def( boost::python::self_ns::str( self ) )
    ;
  class_<program_options, bases<boost::program_options::options_description> >( "program_options", "Program options which can be used for stand-alone tools" )
    .def( "add_costs_option", &program_options::add_costs_option, return_value_policy<reference_existing_object>() )
    .def( "add_read_specification_option", &program_options::add_read_specification_option, return_value_policy<reference_existing_object>() )
    .def( "add_read_realization_option", &program_options::add_read_realization_option, return_value_policy<reference_existing_object>() )
    .def( "add_write_realization_option", &program_options::add_write_realization_option, return_value_policy<reference_existing_object>() )
    .def( "add_numeric_option", program_options_add_option1<unsigned>, return_value_policy<reference_existing_object>() )
    .def( "add_double_option", program_options_add_option1<double>, return_value_policy<reference_existing_object>() )
    .def( "add_option", program_options_add_option1<std::string>, return_value_policy<reference_existing_object>() )
    .def( "add_option", program_options_add_option2<unsigned>, return_value_policy<reference_existing_object>() )
    .def( "add_option", program_options_add_option2<std::string>, return_value_policy<reference_existing_object>() )
    .def( "costs", program_options_costs )
    .def( "good", &program_options::good )
    .def( "is_set", &program_options::is_set )
    .def( "parse", program_options_parse )
    .def( "read_realization_filename", &program_options::read_realization_filename, return_value_policy<copy_const_reference>() )
    .def( "read_specification_filename", &program_options::read_specification_filename, return_value_policy<copy_const_reference>() )
    .def( "write_realization_filename", &program_options::write_realization_filename, return_value_policy<copy_const_reference>() )
    .def( "is_write_realization_filename_set", &program_options::is_write_realization_filename_set )
    .def( "__getitem__", program_options_value )
    ;

}
