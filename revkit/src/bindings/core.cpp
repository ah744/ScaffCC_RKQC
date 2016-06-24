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
#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/extract.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

#include <core/circuit.hpp>
#include <core/gate.hpp>
#include <core/pattern.hpp>
#include <core/properties.hpp>
#include <core/target_tags.hpp>
#include <core/truth_table.hpp>
#include <core/version.hpp>

#include <core/io/print_circuit.hpp>

#include <core/utils/costs.hpp>

#include <algorithms/optimization/optimization.hpp>
#include <algorithms/optimization/line_reduction.hpp>
#include <algorithms/optimization/window_optimization.hpp>

#include <algorithms/simulation/simulation.hpp>
#include <algorithms/simulation/sequential_simulation.hpp>
#include <algorithms/simulation/simple_simulation.hpp>

#include <algorithms/synthesis/synthesis.hpp>
#include <algorithms/synthesis/esop_synthesis.hpp>
#include <algorithms/synthesis/quantum_decomposition.hpp>
#include <algorithms/synthesis/swop.hpp>
#include <algorithms/verification/verification.hpp>

using namespace boost::python;
using namespace revkit;

// CIRCUIT
circuit::const_iterator (circuit::*begin1)() const = &circuit::begin;
circuit::const_iterator (circuit::*end1)() const = &circuit::end;
circuit::iterator (circuit::*begin2)() = &circuit::begin;
circuit::iterator (circuit::*end2)() = &circuit::end;

circuit::const_reverse_iterator (circuit::*rbegin1)() const = &circuit::rbegin;
circuit::const_reverse_iterator (circuit::*rend1)() const = &circuit::rend;
circuit::reverse_iterator (circuit::*rbegin2)() = &circuit::rbegin;
circuit::reverse_iterator (circuit::*rend2)() = &circuit::rend;

bus_collection& (circuit::*inputbuses1)() = &circuit::inputbuses;
bus_collection& (circuit::*outputbuses1)() = &circuit::outputbuses;
bus_collection& (circuit::*statesignals1)() = &circuit::statesignals;
void (circuit::*add_module1)(const std::string&, const circuit&) = &circuit::add_module;

void circuit_append_gate( circuit& c, const gate& g )
{
  c.append_gate() = g;
}

void circuit_prepend_gate( circuit& c, const gate& g )
{
  c.prepend_gate() = g;
}

void circuit_insert_gate( circuit& c, unsigned pos, const gate& g )
{
  c.insert_gate( pos ) = g;
}

dict circuit_modules( const circuit& c )
{
  dict d;

  typedef std::pair<std::string, boost::shared_ptr<circuit> > pair_t;
  foreach ( const pair_t& p, c.modules() )
  {
    d[p.first] = p.second;
  }

  return d;
}

dict circuit_annotations( const circuit& c, const gate& g )
{
  dict d;

  boost::optional<const std::map<std::string, std::string>& > annotations = c.annotations( g );

  if ( annotations )
  {
    typedef std::pair<std::string, std::string> pair_t;
    foreach ( const pair_t& p, *annotations )
    {
      d[p.first] = p.second;
    }
  }

  return d;
}

template<typename T, typename C>
list list_getter( const C& circ, boost::function<const std::vector<T>& (const C*)> fgetter )
{
  list l;
  std::for_each( fgetter( &circ ).begin(), fgetter( &circ ).end(), boost::bind( &list::append<T>, &l, _1 ) );
  return l;
}

template<typename T, typename C>
void list_setter( C& circ, object o, boost::function<void (C*, const std::vector<T>&)> fsetter )
{
  stl_input_iterator<T> begin( o ), end;
  std::vector<T> v;
  std::copy( begin, end, std::back_inserter( v ) );
  fsetter( &circ, v );
}

list inputs_getter( const circuit& circ ) { return list_getter<std::string, circuit>( circ, &circuit::inputs ); }
void inputs_setter( circuit& circ, object o ) { return list_setter<std::string, circuit>( circ, o, &circuit::set_inputs ); }
list outputs_getter( const circuit& circ ) { return list_getter<std::string, circuit>( circ, &circuit::outputs ); }
void outputs_setter( circuit& circ, object o ) { return list_setter<std::string, circuit>( circ, o, &circuit::set_outputs ); }
list constants_getter( const circuit& circ ) { return list_getter<constant, circuit>( circ, &circuit::constants ); }
void constants_setter( circuit& circ, object o ) { return list_setter<constant, circuit>( circ, o, &circuit::set_constants ); }
list garbage_getter( const circuit& circ ) { return list_getter<bool, circuit>( circ, &circuit::garbage ); }
void garbage_setter( circuit& circ, object o ) { return list_setter<bool, circuit>( circ, o, &circuit::set_garbage ); }

const gate& circuit_get_gate( const circuit& circ, const size_t& i )
{
  return *( circ.begin() + i );
}

circuit subcircuit1( const circuit& base, unsigned from, unsigned to )
{
  return subcircuit( base, from, to );
}

circuit subcircuit2( const circuit& base, unsigned from, unsigned to, object list )
{
  stl_input_iterator<unsigned> begin( list ), end;
  std::vector<unsigned> filter( begin, end );
  return subcircuit( base, from, to, filter );
}

list circuit_filter( const circuit& circ )
{
  list l;

  unsigned lines;
  std::vector<unsigned> filter;

  boost::tie( lines, filter ) = circ.filter();

  l.append<unsigned>( lines );

  list lfilter;
  std::for_each( filter.begin(), filter.end(), boost::bind( &list::append<unsigned>, &lfilter, _1 ) );

  l.append<list>( lfilter );

  return l;
}

template<typename K, typename T>
void multimap_to_map( const std::multimap<K, T>& mmap, std::map<K, std::vector<T> >& map )
{
  for ( typename std::multimap<K, T>::const_iterator it = mmap.begin(); it != mmap.end(); ++it )
  {
    if ( map.find( it->first ) == map.end() )
    {
      map.insert( std::make_pair( it->first, std::vector<T>()  ) );
    }

    map[it->first].push_back( it->second );
  }
}

// GATE
// only in python
namespace gate_types {
  enum _types {
    toffoli, peres, fredkin, v, vplus, module
  };
}

gate::const_iterator (gate::*begin_controls1)() = &gate::begin_controls;
gate::iterator (gate::*begin_controls2)() = &gate::begin_controls;
gate::const_iterator (gate::*end_controls1)() = &gate::end_controls;
gate::iterator (gate::*end_controls2)() = &gate::end_controls;

gate::const_iterator (gate::*begin_targets1)() = &gate::begin_targets;
gate::iterator (gate::*begin_targets2)() = &gate::begin_targets;
gate::const_iterator (gate::*end_targets1)() = &gate::end_targets;
gate::iterator (gate::*end_targets2)() = &gate::end_targets;

void gate_set_type( gate& g, unsigned value )
{
  switch ( value )
  {
  case gate_types::toffoli:
    g.set_type( toffoli_tag() );
    break;
  case gate_types::peres:
    g.set_type( peres_tag() );
    break;
  case gate_types::fredkin:
    g.set_type( fredkin_tag() );
    break;
  case gate_types::v:
    g.set_type( v_tag() );
    break;
  case gate_types::vplus:
    g.set_type( vplus_tag() );
    break;
  case gate_types::module:
    g.set_type( module_tag() );
    break;
  default:
    assert( false );
    break;
  }
}

unsigned gate_get_type( const gate& g )
{
  if ( is_toffoli( g ) )
  {
    return gate_types::toffoli;
  }
  else if ( is_peres( g ) )
  {
    return gate_types::peres;
  }
  else if ( is_fredkin( g ) )
  {
    return gate_types::fredkin;
  }
  else if ( is_v( g ) )
  {
    return gate_types::v;
  }
  else if ( is_vplus( g ) )
  {
    return gate_types::vplus;
  }
  else if ( is_module( g ) )
  {
    return gate_types::module;
  }

  assert( false );
}

std::string gate_module_name( const gate& g )
{
  if ( is_module( g ) )
  {
    return boost::any_cast<module_tag>( g.type() ).name;
  }
  else
  {
    return std::string();
  }
}

const circuit& gate_module_reference( const gate& g )
{
  assert( is_module( g ) );

  return *boost::any_cast<module_tag>( g.type() ).reference;
}

unsigned gate_num_controls( const gate& g )
{
  return std::distance( g.begin_controls(), g.end_controls() );
}

unsigned gate_num_targets( const gate& g )
{
  return std::distance( g.begin_targets(), g.end_targets() );
}

bool binary_truth_table_add_entry( binary_truth_table& spec, object in, object out )
{
  stl_input_iterator<constant> begin1( in ), begin2( out ), end;

  binary_truth_table::cube_type in_cube, out_cube;

  std::copy( begin1, end, std::back_inserter( in_cube ) );
  std::copy( begin2, end, std::back_inserter( out_cube ) );

  return spec.add_entry( in_cube, out_cube );
}

list permutation_getter( const binary_truth_table& circ ) { return list_getter<unsigned, binary_truth_table>( circ, &binary_truth_table::permutation ); }
void permutation_setter( binary_truth_table& circ, object o ) { return list_setter<unsigned, binary_truth_table>( circ, o, &binary_truth_table::set_permutation ); }

list binary_truth_table_inputs_getter( const binary_truth_table& spec ) { return list_getter<std::string, binary_truth_table>( spec, &binary_truth_table::inputs ); }
void binary_truth_table_inputs_setter( binary_truth_table& spec, object o ) { return list_setter<std::string, binary_truth_table>( spec, o, &binary_truth_table::set_inputs ); }
list binary_truth_table_outputs_getter( const binary_truth_table& spec ) // different, because it returns copy and no reference
{
  list l;
  std::vector<std::string> outputs = spec.outputs();
  std::for_each( outputs.begin(), outputs.end(), boost::bind( &list::append<std::string>, &l, _1 ) );
  return l;
}
void binary_truth_table_outputs_setter( binary_truth_table& spec, object o ) { return list_setter<std::string, binary_truth_table>( spec, o, &binary_truth_table::set_outputs ); }
list binary_truth_table_constants_getter( const binary_truth_table& spec ) { return list_getter<constant, binary_truth_table>( spec, &binary_truth_table::constants ); }
void binary_truth_table_constants_setter( binary_truth_table& spec, object o ) { return list_setter<constant, binary_truth_table>( spec, o, &binary_truth_table::set_constants ); }
list binary_truth_table_garbage_getter( const binary_truth_table& spec ) // different, because it returns copy and no reference
{
  list l;
  std::vector<bool> garbage = spec.garbage();
  std::for_each( garbage.begin(), garbage.end(), boost::bind( &list::append<bool>, &l, _1 ) );
  return l;
}
void binary_truth_table_garbage_setter( binary_truth_table& spec, object o ) { return list_setter<bool, binary_truth_table>( spec, o, &binary_truth_table::set_garbage ); }

struct binary_truth_table_entry_to_python
{
  static PyObject* convert( const transform_cube<constant>::result_type& value )
  {
    list l;

    list in;
    std::for_each( value.first.first, value.first.second, boost::bind( &list::append<constant>, &in, _1 ) );

    list out;
    std::for_each( value.second.first, value.second.second, boost::bind( &list::append<constant>, &out, _1 ) );

    l.append<list>( in );
    l.append<list>( out );

    return incref( l.ptr() );
  }
};


template<typename T>
void properties_set( properties& prop, const std::string& key, const T& value )
{
  prop.set( key, value );
}

template<typename T>
T properties_get1( properties& prop, const std::string& key )
{
  return prop.get<T>( key );
}

template<typename T>
T properties_get2( properties& prop, const std::string& key, const T& default_value )
{
  return prop.get<T>( key, default_value );
}

template<typename T>
void properties_set_vector( properties& prop, const std::string& key, list o )
{
  stl_input_iterator<T> begin( o ), end;
  std::vector<T> value( begin, end );
  prop.set( key, value );
}

template<typename T>
list properties_get_vector1( properties& prop, const std::string& key )
{
  std::vector<T> value = prop.get<std::vector<T> >( key );
  list l;
  std::for_each( value.begin(), value.end(), boost::bind( &list::append<T>, l, _1 ) );
  return l;
}

template<typename T>
list properties_get_vector2( properties& prop, const std::string& key, const std::vector<T>& default_value )
{
  std::vector<T> value = prop.get<std::vector<T> >( key, default_value );
  list l;
  std::for_each( value.begin(), value.end(), boost::bind( &list::append<T>, l, _1 ) );
  return l;
}

template<typename T>
void properties_set_cost_function( properties& prop, const std::string& key, const T& f )
{
  prop.set( key, cost_function( f ) );
}

list properties_get_counterexample( properties& prop, const std::string& key )
{
  counterexample value = prop.get<counterexample>( key );
  list l;
  typedef counterexample::value_type value_type;
  foreach ( value_type const& cex, value )
  {
    list l2;
    l2.append<boost::dynamic_bitset<> > (cex.first);
    l2.append<boost::dynamic_bitset<> > (cex.second);

    l.append<list> (l2);
  }
  return l;
}

void properties_set_line_mapping( properties& prop, const std::string& key, dict data )
{
  typedef std::map<std::string, std::string> mapping_t;

  stl_input_iterator<object> begin( data.keys() ), end;
  std::vector<object> v;
  std::copy( begin, end, std::back_inserter( v ) );

  mapping_t mapping;
  for ( std::vector<object>::const_iterator iter = v.begin(); iter != v.end(); ++iter )
  {
     std::string f1 = extract<std::string>( *iter );
     std::string f2 = extract<std::string>( data[*iter] );
     mapping.insert( std::make_pair( f1, f2 ) );
  }

  prop.set( key, mapping );
}

void properties_set_bitset_map( properties& prop, const std::string& key, dict data )
{
  std::map<std::string, boost::dynamic_bitset<> > map;

  stl_input_iterator<std::string> begin( data.keys() ), end;

  foreach ( const std::string& key, std::make_pair( begin, end ) )
  {
    list l = extract<list>( data[key] );

    stl_input_iterator<bool> _begin( l ), _end;
    std::vector<bool> bits;
    std::copy( _begin, _end, std::back_inserter( bits ) );

    boost::dynamic_bitset<> bitset( bits.size() );
    for ( unsigned i = 0u; i < bits.size(); ++i )
    {
      bitset.set( i, bits.at( i ) );
    }

    map.insert( std::make_pair( key, bitset ) );
  }

  prop.set( key, map );
}

std::string bitset_to_string (boost::dynamic_bitset<> const& bitset)
{
  std::string res;
  for (unsigned i = 0; i < bitset.size(); i++)
  {
    res += bitset[i] ? "1" : "0";
  }
  return res;
}

boost::range_iterator<boost::range_detail::select_first_range<bus_collection::map> >::type bus_collection_names_begin( const bus_collection& b )
{
  using boost::adaptors::map_keys;
  using boost::begin;

  return begin( b.buses() | map_keys );
}

boost::range_iterator<boost::range_detail::select_first_range<bus_collection::map> >::type bus_collection_names_end( const bus_collection& b )
{
  using boost::adaptors::map_keys;
  using boost::end;

  return end( b.buses() | map_keys );
}

object bus_collection_get( const bus_collection& b, const std::string& name )
{
  list l;
  boost::for_each( b.get( name ), boost::bind( &list::append<unsigned>, l, _1 ) );
  return l;
}

void bus_collection_add( bus_collection& b, const std::string& name, object l )
{
  stl_input_iterator<unsigned> begin( l ), end;
  std::vector<unsigned> list( begin, end );

  b.add( name, list );
}

bool bus_collection_empty( const bus_collection& b )
{
  return b.buses().empty();
}

boost::dynamic_bitset<>& (boost::dynamic_bitset<>::*set1)(boost::dynamic_bitset<>::size_type, bool) = &boost::dynamic_bitset<>::set;
boost::dynamic_bitset<>& (boost::dynamic_bitset<>::*set2)() = &boost::dynamic_bitset<>::set;
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( set1_overloads, set, 0, 2 )

// Pattern
dict pattern_initializers( const pattern& p )
{
  dict r;

  foreach ( const pattern::initializer_map::value_type& pair, p.initializers() )
  {
    r[pair.first] = pair.second;
  }

  return r;
}

list pattern_inputs( const pattern& p )
{
  list r;
  std::for_each( p.inputs().begin(), p.inputs().end(), boost::bind( &list::append<std::string>, r, _1 ) );
  return r;
}

list pattern_patterns( const pattern& p )
{
  list r;

  foreach ( const pattern::pattern_t& v, p.patterns() )
  {
    list inner;
    std::for_each( v.begin(), v.end(), boost::bind( &list::append<unsigned>, inner, _1 ) );
    r.append( inner );
  }

  return r;
}

void export_core()
{

  def( "revkit_version", &revkit_version );

  class_<bus_collection>( "bus_collection", "Generic collection class for bus information" )
    .def( "__getitem__", bus_collection_get )
    .def( "__iter__", boost::python::range( bus_collection_names_begin, bus_collection_names_end ) )
    .def( "add", &bus_collection_add )
    .def( "find_bus", &bus_collection::find_bus )
    .def( "has_bus", &bus_collection::has_bus )
    .def( "signal_index", &bus_collection::signal_index )
    .def( "empty", &bus_collection_empty )
    ;

  class_<circuit, boost::shared_ptr<circuit> >( "circuit", "A circuit is a collection of gates with meta-data", init<>() )
    .def( init<unsigned>() )

    .add_property( "lines", &circuit::lines, &circuit::set_lines )
    .add_property( "num_gates", &circuit::num_gates )
    .add_property( "gates", boost::python::range<return_value_policy<reference_existing_object> >( begin1, end1 ) )
    .add_property( "rgates", boost::python::range<return_value_policy<reference_existing_object> >( rbegin1, rend1 ) )
    .add_property( "inputs", inputs_getter, inputs_setter )
    .add_property( "outputs", outputs_getter, outputs_setter )
    .add_property( "constants", constants_getter, constants_setter )
    .add_property( "garbage", garbage_getter, garbage_setter )
    .add_property( "circuit_name", make_function( &circuit::circuit_name, return_value_policy<copy_const_reference>() ), &circuit::set_circuit_name )
    .add_property( "filter", circuit_filter )
    .add_property( "offset", &circuit::offset )

    .def( "append_gate", circuit_append_gate )
    .def( "prepend_gate", circuit_prepend_gate )
    .def( "insert_gate", circuit_insert_gate )
    .def( "remove_gate_at", &circuit::remove_gate_at )
    .def( "is_subcircuit", &circuit::is_subcircuit )
    .def( "inputbuses", inputbuses1, return_internal_reference<1>() )
    .def( "outputbuses", outputbuses1, return_internal_reference<1>() )
    .def( "statesignals", statesignals1, return_internal_reference<1>() )
    .def( "add_module", add_module1 )
    .def( "modules", circuit_modules )
    .def( "annotation", &circuit::annotation, return_value_policy<copy_const_reference>() )
    .def( "annotations", circuit_annotations )
    .def( "annotate", &circuit::annotate )

    .def( boost::python::self_ns::str( self ) )
    .def( boost::python::self_ns::repr( self ) )
    .def( "__getitem__", circuit_get_gate, return_value_policy<reference_existing_object>() )
    .def( "__iter__", boost::python::range<return_value_policy<reference_existing_object> >( begin1, end1 ) )
    ;
  def( "subcircuit", subcircuit1 );
  def( "subcircuit", subcircuit2 );

  enum_<gate_types::_types>( "gate_type" )
    .value( "toffoli", gate_types::toffoli )
    .value( "peres", gate_types::peres )
    .value( "fredkin", gate_types::fredkin )
    .value( "v", gate_types::v )
    .value( "vplus", gate_types::vplus )
    .value( "module", gate_types::module )
    ;

  class_<gate>( "gate", "Gate consisting of control and target lines" )
    .add_property( "controls", boost::python::range( begin_controls1, end_controls1 ) )
    .add_property( "targets", boost::python::range( begin_targets1, end_targets1 ) )
    .add_property( "size", &gate::size )
    .add_property( "num_controls", gate_num_controls )
    .add_property( "num_targets", gate_num_targets )
    .add_property( "type", gate_get_type, gate_set_type )
    .add_property( "module_name", gate_module_name )
    .add_property( "module_reference", make_function( gate_module_reference, return_value_policy<reference_existing_object>() ) )

    .def( "add_control", &gate::add_control )
    .def( "remove_control", &gate::remove_control )
    .def( "add_target", &gate::add_target )
    .def( "remove_target", &gate::remove_target )
    ;

  boost::python::to_python_converter<transform_cube<constant>::result_type, binary_truth_table_entry_to_python>();
  class_<binary_truth_table>( "binary_truth_table", "A truth table for binary values" )
    .def( boost::python::self_ns::str( self ) )
    .def( boost::python::self_ns::repr( self ) )

    .add_property( "entries", boost::python::range( &binary_truth_table::begin, &binary_truth_table::end ) )
    .add_property( "num_inputs", &binary_truth_table::num_inputs )
    .add_property( "num_outputs", &binary_truth_table::num_outputs )
    .add_property( "permutation", permutation_getter, permutation_setter )
    .add_property( "inputs", binary_truth_table_inputs_getter, binary_truth_table_inputs_setter )
    .add_property( "outputs", binary_truth_table_outputs_getter, binary_truth_table_outputs_setter )
    .add_property( "constants", binary_truth_table_constants_getter, binary_truth_table_constants_setter )
    .add_property( "garbage", binary_truth_table_garbage_getter, binary_truth_table_garbage_setter )

    .def( "add_entry", binary_truth_table_add_entry )
    .def( "clear", &binary_truth_table::clear )
    .def( "permute", &binary_truth_table::permute )
    ;

  class_<properties, boost::shared_ptr<properties> >( "properties", "Properties are used to specify parameters for algorithm calls", init<>() )
    .def( "set_string", &properties_set<std::string> )
    .def( "set_bool", &properties_set<bool> )
    .def( "set_int", &properties_set<int> )
    .def( "set_unsigned", &properties_set<unsigned> )
    .def( "set_double", &properties_set<double> )
    .def( "set_char", &properties_set<char> )
    .def( "set_bitset_map", &properties_set_bitset_map )
    .def( "set_cost_function", &properties_set_cost_function<costs_by_circuit_func> )
    .def( "set_cost_function", &properties_set_cost_function<costs_by_gate_func> )
    .def( "set_truth_table_synthesis_func", &properties_set<truth_table_synthesis_func> )
    .def( "set_gate_decomposition_func", &properties_set<gate_decomposition_func> )
    .def( "set_swop_step_func", &properties_set<swop_step_func> )
    .def( "set_simulation_func", &properties_set<simulation_func> )
    .def( "set_window_synthesis_func", &properties_set<window_synthesis_func> )
    .def( "set_optimization_func", &properties_set<optimization_func> )
    .def( "set_select_window_func", &properties_set<select_window_func> )
    .def( "set_step_result_func", &properties_set<step_result_func> )
    .def( "set_sequential_step_result_func", &properties_set<sequential_step_result_func> )
    .def( "set_cube_reordering_func", &properties_set<cube_reordering_func> )
    .def( "set_vector_unsigned", &properties_set_vector<unsigned> )
    .def( "set_line_mapping", &properties_set_line_mapping )

    .def( "get_string", &properties_get1<std::string> )
    .def( "get_bool", &properties_get1<bool> )
    .def( "get_int", &properties_get1<int> )
    .def( "get_unsigned", &properties_get1<unsigned> )
    .def( "get_double", &properties_get1<double> )
    .def( "get_char", &properties_get1<char> )
    .def( "get_bitset_map", &properties_get1<std::map<std::string, boost::dynamic_bitset<> > > )
    .def( "get_truth_table_synthesis_func", &properties_get1<truth_table_synthesis_func> )
    .def( "get_gate_decomposition_func", &properties_get1<gate_decomposition_func> )
    .def( "get_swop_step_func", &properties_get1<swop_step_func> )
    .def( "get_simulation_func", &properties_get1<simulation_func> )
    .def( "get_window_synthesis_func", &properties_get1<window_synthesis_func> )
    .def( "get_optimization_func", &properties_get1<optimization_func> )
    .def( "get_select_window_func", &properties_get1<select_window_func> )
    .def( "get_step_result_func", &properties_get1<step_result_func> )
    .def( "get_sequential_step_result_func", &properties_get1<sequential_step_result_func> )
    .def( "get_cube_reordering_func", &properties_get1<cube_reordering_func> )
    .def( "get_vector_unsigned", &properties_get_vector1<unsigned> )
    .def( "get_counterexample", &properties_get_counterexample )

    .def( "get_string", &properties_get2<std::string> )
    .def( "get_bool", &properties_get2<bool> )
    .def( "get_int", &properties_get2<int> )
    .def( "get_unsigned", &properties_get2<unsigned> )
    .def( "get_double", &properties_get2<double> )
    .def( "get_char", &properties_get2<char> )
    .def( "get_bitset_map", &properties_get2<std::map<std::string, boost::dynamic_bitset<> > > )
    .def( "get_truth_table_synthesis_func", &properties_get2<truth_table_synthesis_func> )
    .def( "get_gate_decomposition_func", &properties_get2<gate_decomposition_func> )
    .def( "get_swop_step_func", &properties_get2<swop_step_func> )
    .def( "get_simulation_func", &properties_get2<simulation_func> )
    .def( "get_window_synthesis_func", &properties_get2<window_synthesis_func> )
    .def( "get_optimization_func", &properties_get2<optimization_func> )
    .def( "get_select_window_func", &properties_get2<select_window_func> )
    .def( "get_step_result_func", &properties_get2<step_result_func> )
    .def( "get_sequential_step_result_func", &properties_get2<sequential_step_result_func> )
    .def( "get_cube_reordering_func", &properties_get2<cube_reordering_func> )
    .def( "get_vector_unsigned", &properties_get_vector2<unsigned> )
    ;

  class_<boost::dynamic_bitset<> >( "bitset", init<>() )
    .def( init<boost::dynamic_bitset<>::size_type>() )
    .def( init<boost::dynamic_bitset<>::size_type, unsigned long>() )
    .def( "set", set1, set1_overloads()[ return_value_policy<reference_existing_object>() ] )
    .def( "set", set2, return_value_policy<reference_existing_object>() )
    .def( "test", &boost::dynamic_bitset<>::test )
    .def( "to_ulong", &boost::dynamic_bitset<>::to_ulong )
    .def( "size", &boost::dynamic_bitset<>::size )
    .def( "resize", &boost::dynamic_bitset<>::resize )
    .def( "clear", &boost::dynamic_bitset<>::clear )
    //    .def( "reset", &boost::dynamic_bitset<>::reset )
    .def( "__str__", bitset_to_string)
    .def( "__repr__", bitset_to_string)
    ;

  class_<pattern>( "pattern" )
    .add_property( "initializers", &pattern_initializers )
    .add_property( "inputs", &pattern_inputs )
    .add_property( "patterns", &pattern_patterns )
    ;

}
