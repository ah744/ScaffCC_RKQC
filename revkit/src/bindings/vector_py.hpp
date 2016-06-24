/* This code is initially taken from VectorPy.h from John Hunter */

#ifndef VECTOR_PY_HPP
#define VECTOR_PY_HPP

#include <boost/python.hpp>

#include <string>
#include <vector>

template<typename T>
class python_vector : public boost::python::class_<std::vector<T> >
{
public:
  python_vector( const std::string& class_name );
};

#endif /* VECTOR_PY_HPP */
