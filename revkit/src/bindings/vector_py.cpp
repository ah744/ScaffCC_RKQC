#include "vector_py.hpp"

#include <sstream>
#include <string>

#include <boost/python/stl_iterator.hpp>

#include <core/circuit.hpp> 

template<typename T>
void v_push_back( std::vector<T>& v, const T& x )
{
  v.push_back(x);
}

template<typename T>
T v_get_item( const std::vector<T>& v, const size_t& i )
{
  return v.at( i );
}

template<typename T>
void v_set_item( std::vector<T>& v, const size_t& i, const T& val )
{
  v.at( i ) = val;
}

template<typename T>
void v_resize( std::vector<T>& v, const size_t& i )
{
  v.resize( i );
}

template<typename T>
void v_clear( std::vector<T>& v )
{
  v.clear();
}

template<typename T>
size_t v_size( const std::vector<T>& v )
{
  return v.size();
}

template<typename T>
std::string v_print( const std::vector<T>& v )
{
  std::stringstream oss;
  oss << '[';

  for ( unsigned i = 0; i < v.size(); ++i )
  {
    oss << v.at( i );
    if ( i != v.size() - 1 )
    {
      oss << ", ";
    }
  }

  oss << ']';

  return oss.str();
}

template<typename T>
void v_assign( std::vector<T>& v, boost::python::object o )
{
  boost::python::stl_input_iterator<T> begin( o ), end;
  v.assign( begin, end );
}

template<typename T>
python_vector<T>::python_vector(const std::string& class_name )
  : boost::python::class_<std::vector<T> >( class_name.c_str(), boost::python::init<>() )
{
  
  this->def( boost::python::init<size_t,T>() );
  this->def( "push_back", v_push_back<T> );
  this->def( "size", v_size<T> );
  this->def( "resize", v_resize<T> );
  this->def( "clear", v_clear<T> );
  this->def( "__getitem__", v_get_item<T> );
  this->def( "__setitem__", v_set_item<T> );
  this->def( "__iter__", boost::python::iterator<std::vector<T> >() );
  this->def( "__str__", v_print<T> );
  this->def( "assign", &v_assign<T> );

}

//template instantiations
#define INSTANTIATE_TEMPLATE(T) \
template class std::vector<T>;\
template void v_push_back( std::vector<T>&, const T& ); \
template void v_resize( std::vector<T>&, const size_t& ); \
template void v_clear( std::vector<T>& );                 \
template size_t v_size( const std::vector<T>& );              \
template T v_get_item( const std::vector<T>&, const size_t& );      \
template void v_set_item( std::vector<T>&, const size_t&, const T& ); \
template std::string v_print( const std::vector<T>& );                \
template void v_assign( std::vector<T>&, boost::python::object o ); \
template class python_vector<T>;

INSTANTIATE_TEMPLATE(std::string)
INSTANTIATE_TEMPLATE(bool)
INSTANTIATE_TEMPLATE(revkit::constant)

