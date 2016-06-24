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
#include <boost/python/stl_iterator.hpp>
#include <boost/range/irange.hpp>

#include <core/pattern.hpp>

#include <core/io/create_image.hpp>
#include <core/io/print_circuit.hpp>
#include <core/io/print_statistics.hpp>
#include <core/io/read_pattern.hpp>
#include <core/io/read_pla.hpp>
#include <core/io/read_realization.hpp>
#include <core/io/read_specification.hpp>
#include <core/io/write_blif.hpp>
#include <core/io/write_realization.hpp>
#include <core/io/write_specification.hpp>
#include <core/io/write_verilog.hpp>

using namespace boost::python;
using namespace revkit;

std::string create_image1( const circuit& circ, create_image_settings& settings )
{
  std::stringstream oss;
  create_image( oss, circ, settings );
  return oss.str();
}

class create_image_settings_wrap : public create_image_settings, public wrapper<create_image_settings>
{
public:
  void draw_begin( std::ostream& os ) const
  {
    this->get_override( "draw_begin" )( os );
  }

  void draw_line( std::ostream& os, float x1, float x2, float y ) const
  {
    this->get_override( "draw_line" )( os, x1, x2, y );
  }

  void draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const
  {
    this->get_override( "draw_input" )( os, x, y, text, is_constant );
  }

  void draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const
  {
    this->get_override( "draw_output" )( os, x, y, text, is_garbage );
  }

  void draw_control( std::ostream& os, float x, float y ) const
  {
    this->get_override( "draw_control" )( os, x, y );
  }

  void draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const
  {
    this->get_override( "draw_targets" )( os, x, ys, target_tag );
  }

  void draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const
  {
    this->get_override( "draw_peres_frame" )( os, x1, y1, x2, y2 );
  }

  void draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const
  {
    this->get_override( "draw_gate_line" )( os, x, y1, y2 );
  }

  void draw_end( std::ostream& os ) const
  {
    this->get_override( "draw_end" )( os );
  }
};

BOOST_PYTHON_FUNCTION_OVERLOADS( print_circuit_overloads, print_circuit, 1, 2 )

void (*print_statistics1)(const circuit&, double runtime, const print_statistics_settings&) = print_statistics;
BOOST_PYTHON_FUNCTION_OVERLOADS( print_statistics1_overloads, print_statistics, 1, 3 )

object py_read_pattern( pattern& p, const std::string& filename )
{
  std::string error;
  if ( read_pattern( p, filename, &error ) )
  {
    return object( true );
  }
  else
  {
    return object( error );
  }
}

bool (*read_pla1)(binary_truth_table&, const std::string&, const read_pla_settings&, std::string*) = read_pla;
BOOST_PYTHON_FUNCTION_OVERLOADS( read_pla1_overloads, read_pla, 2, 4 )

object py_read_realization( circuit& circ, const std::string& filename )
{
  std::string error;
  if ( read_realization( circ, filename, &error ) )
  {
    return object( true );
  }
  else
  {
    return object( error );
  }
}
//bool (*read_realization1)(circuit&, const std::string&, std::string*) = read_realization;
//BOOST_PYTHON_FUNCTION_OVERLOADS( read_realization1_overloads, read_realization, 2, 3 )

object py_read_specification( binary_truth_table& spec, const std::string& filename )
{
  std::string error;
  if ( read_specification( spec, filename, &error ) )
  {
    return object( true );
  }
  else
  {
    return object( error );
  }
}
//bool (*read_specification1)(binary_truth_table&, const std::string&, std::string*) = read_specification;
//BOOST_PYTHON_FUNCTION_OVERLOADS( read_specification1_overloads, read_specification, 2, 3 )

void write_blif1( const circuit& circ, const std::string& filename, const write_blif_settings& settings )
{
  std::ofstream os( filename.c_str() );
  write_blif( circ, os, settings );
}

void write_verilog1( const circuit& circ, const std::string& filename, const write_verilog_settings& settings )
{
  std::ofstream os( filename.c_str() );
  write_verilog( circ, os, settings );
}

bool (*write_realization1)(const circuit&, const std::string&, const write_realization_settings&, std::string*) = write_realization;
BOOST_PYTHON_FUNCTION_OVERLOADS( write_realization1_overloads, write_realization, 2, 4 )

bool (*write_specification1)(const binary_truth_table&, const std::string&, const write_specification_settings&) = write_specification;
BOOST_PYTHON_FUNCTION_OVERLOADS( write_specification1_overloads, write_specification, 2, 3 )

list write_specification_settings_get_output_order( const write_specification_settings& settings )
{
  list l;
  std::for_each( settings.output_order.begin(), settings.output_order.end(), boost::bind( &list::append<unsigned>, l, _1 ) );
  return l;
}

void write_specification_settings_set_output_order( write_specification_settings& settings, object o )
{
  stl_input_iterator<unsigned> begin( o ), end;
  settings.output_order.clear();
  std::copy( begin, end, std::back_inserter( settings.output_order ) );
}

void export_core_io()
{

  // core/io
  class_<create_image_settings_wrap, boost::noncopyable>( "create_image_settings" )
    .def_readwrite( "width", &create_image_settings::width )
    .def_readwrite( "height", &create_image_settings::height )
    .def_readwrite( "elem_width", &create_image_settings::elem_width )
    .def_readwrite( "elem_height", &create_image_settings::elem_height )
    .def_readwrite( "line_width", &create_image_settings::line_width )
    .def_readwrite( "control_radius", &create_image_settings::control_radius )
    .def_readwrite( "target_radius", &create_image_settings::target_radius )

    .def_readwrite( "draw_before_text", &create_image_settings::draw_before_text )
    .def_readwrite( "draw_in_between_text", &create_image_settings::draw_in_between_text )
    .def_readwrite( "draw_after_text", &create_image_settings::draw_after_text )
    ;
  class_<create_pstricks_settings, bases<create_image_settings> >( "create_pstricks_settings" )
    .def_readwrite( "math_emph", &create_pstricks_settings::math_emph )
    ;
  class_<create_tikz_settings, bases<create_image_settings> >( "create_tikz_settings" )
    .def_readwrite( "math_emph", &create_tikz_settings::math_emph )
    ;
  def( "py_create_image", create_image1 );

  class_<print_circuit_settings>( "print_circuit_settings" )
    .def_readwrite( "print_inputs_and_outputs", &print_circuit_settings::print_inputs_and_outputs )
    .def_readwrite( "print_gate_index", &print_circuit_settings::print_gate_index )
    .def_readwrite( "control_char", &print_circuit_settings::control_char )
    .def_readwrite( "line_char", &print_circuit_settings::line_char )
    .def_readwrite( "gate_spacing", &print_circuit_settings::gate_spacing )
    .def_readwrite( "line_spacing", &print_circuit_settings::line_spacing )
    ;
  def( "py_print_circuit", print_circuit, print_circuit_overloads() );
  class_<print_statistics_settings>( "print_statistics_settings" )
    .def_readwrite( "main_template", &print_statistics_settings::main_template )
    .def_readwrite( "runtime_template", &print_statistics_settings::runtime_template )
    ;
  def( "py_print_statistics", print_statistics1, print_statistics1_overloads() );

  def( "py_read_pattern", py_read_pattern );
  def( "py_read_pla", read_pla1, read_pla1_overloads() );
  /*def( "py_read_realization",
       read_realization1,
       read_realization1_overloads( args( "circ", "filename", "error" ), "Read-in Method for RevLib realization files into a circuit" )
       );*/
  def( "py_read_realization", py_read_realization );
  def( "py_read_specification", py_read_specification );
  class_<read_pla_settings>( "read_pla_settings" )
    .def_readwrite( "extend", &read_pla_settings::extend )
  ;

  class_<write_blif_settings>( "write_blif_settings" )
    .def_readwrite( "tmp_signal_name", &write_blif_settings::tmp_signal_name )
    .def_readwrite( "blif_mv", &write_blif_settings::blif_mv )
    .def_readwrite( "state_prefix", &write_blif_settings::state_prefix )
    .def_readwrite( "keep_constant_names", &write_blif_settings::keep_constant_names )
    ;
  def( "py_write_blif", write_blif1 );

  class_<write_verilog_settings>( "write_verilog_settings" )
    .def_readwrite( "propagate_constants", &write_verilog_settings::propagate_constants )
    ;
  def( "py_write_verilog", write_verilog1 );

  class_<write_realization_settings>( "write_realization_settings" )
    .def_readwrite( "version", &write_realization_settings::version )
    .def_readwrite( "header", &write_realization_settings::header )
    ;
  def( "py_write_realization", write_realization1, write_realization1_overloads() );

  class_<write_specification_settings>( "write_specification_settings" )
    .def_readwrite( "version", &write_specification_settings::version )
    .def_readwrite( "header", &write_specification_settings::header )
    .add_property( "output_order", write_specification_settings_get_output_order, write_specification_settings_set_output_order )
    ;
  def( "py_write_specification", write_specification1, write_specification1_overloads() );

}
