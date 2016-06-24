// boost::optional<T> to/from python converter from John Wiegley

#ifndef PY_OPTIONAL_HPP
#define PY_OPTIONAL_HPP

#include <boost/python.hpp>

#include <iostream>

template <typename T, typename TfromPy>
struct object_from_python
{
  object_from_python()
  {
    boost::python::converter::registry::push_back( &TfromPy::convertible,
                                                   &TfromPy::construct,
                                                   boost::python::type_id<T>() );
  }
};

template <typename T, typename TtoPy, typename TfromPy>
struct register_python_conversion
{
  register_python_conversion()
  {
    boost::python::to_python_converter<T, TtoPy>();
    object_from_python<T, TfromPy>();
  }
};

template<typename T>
PyObject* hook_convert( T value )
{
  return boost::python::to_python_value<T>()( value );
}

template<>
PyObject* hook_convert( bool value )
{
  if ( value )
  {
    Py_RETURN_TRUE;
  }
  else
  {
    Py_RETURN_FALSE;
  }
}

template <typename T>
struct python_optional : public boost::noncopyable
{
  struct optional_to_python
  {
    static PyObject* convert( const boost::optional<T>& value )
    {
      return ( value ? hook_convert( *value )
               : boost::python::detail::none() );
    }
  };

  struct optional_from_python
  {
    static void* convertible( PyObject * source )
    {
      using namespace boost::python::converter;
      
      if (source == Py_None)
      {
        return source;
      }

      const registration& converters( registered<T>::converters );
      
      if ( implicit_rvalue_convertible_from_python( source, converters) )
      {
        rvalue_from_python_stage1_data data = rvalue_from_python_stage1( source, converters );
        return rvalue_from_python_stage2( source, data, converters );
       }
      return 0;
    }

    static void construct( PyObject * source,
                           boost::python::converter::rvalue_from_python_stage1_data * data )
    {
      using namespace boost::python::converter;

      void* const storage = ( (rvalue_from_python_storage<T>*)data )->storage.bytes;

      if (data->convertible == source) // == None
        new (storage) boost::optional<T>(); // A Boost uninitialized value
      else
      {
	T b = *static_cast<T*>( data->convertible );
        new (storage) boost::optional<T>( b );
      }

      data->convertible = storage;
    }
  };

  explicit python_optional()
  {
    register_python_conversion<boost::optional<T>, optional_to_python, optional_from_python>();
  }
};

#endif /* PY_OPTIONAL_HPP */
