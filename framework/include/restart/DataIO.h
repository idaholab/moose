/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DATAIO_H
#define DATAIO_H

#include "Moose.h"
#include "ColumnMajorMatrix.h"

//libMesh
#include "libmesh/dense_matrix.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class MooseMesh;
class FEProblem;

/**
 * Scalar helper routine
 */
template<typename P>
void storeHelper(std::ostream & stream, P & data, void * context);

/**
 * Vector helper routine
 */
template<typename P>
void storeHelper(std::ostream & stream, std::vector<P> & data, void * context);

/**
 * Set helper routine
 */
template<typename P>
void storeHelper(std::ostream & stream, std::set<P> & data, void * context);

/**
 * Map helper routine
 */
template<typename P, typename Q>
void storeHelper(std::ostream & stream, std::map<P,Q> & data, void * context);

/**
 * Scalar helper routine
 */
template<typename P>
void loadHelper(std::istream & stream, P & data, void * context);

/**
 * Vector helper routine
 */
template<typename P>
void loadHelper(std::istream & stream, std::vector<P> & data, void * context);

/**
 * Set helper routine
 */
template<typename P>
void loadHelper(std::istream & stream, std::set<P> & data, void * context);

/**
 * Map helper routine
 */
template<typename P, typename Q>
void loadHelper(std::istream & stream, std::map<P,Q> & data, void * context);

// global store functions

template<typename T>
void dataStore(std::ostream & stream, T & v, void * /*context*/)
{
  // std::cout<<"Generic dataStore"<<std::endl;

  stream.write((char *) &v, sizeof(v));
}

template<typename T>
void dataStore(std::ostream & /*stream*/, T * & /*v*/, void * /*context*/)
{
  mooseError("Cannot store raw pointers as restartable data!\nWrite a custom dataStore() template specialization!\n\n");
}

template<>
inline
void dataStore(std::ostream & stream, Real & v, void * /*context*/)
{
  // std::cout<<"Real dataStore"<<std::endl;

  stream.write((char *) &v, sizeof(v));
}

template<typename T>
inline void
dataStore(std::ostream & stream, const std::vector<T> & v, void * context)
{
  // std::cout<<"Vector dataStore"<<std::endl;

  // First store the size of the vector
  unsigned int size = v.size();
  stream.write((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
    storeHelper(stream, v[i], context);
}

template<typename T>
inline void
dataStore(std::ostream & stream, const std::set<T> & s, void * context)
{
  // std::cout<<"Set dataStore"<<std::endl;

  // First store the size of the set
  unsigned int size = s.size();
  stream.write((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  typename std::set<T>::const_iterator it = s.begin();
  typename std::set<T>::const_iterator end = s.end();

  for (; it != end; ++it)
    storeHelper(stream, *it, context);
}

template<typename T, typename U>
inline void
dataStore(std::ostream & stream, const std::map<T,U> & m, void * context)
{
  // std::cout<<"Map dataStore"<<std::endl;

  // First store the size of the map
  unsigned int size = m.size();
  stream.write((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  typename std::map<T,U>::const_iterator it = m.begin();
  typename std::map<T,U>::const_iterator end = m.end();

  for (; it != end; ++it)
  {
    storeHelper(stream, it->first, context);
    storeHelper(stream, it->second, context);
  }
}

template<>
inline void
dataStore(std::ostream & stream, DenseMatrix<Real> & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((char *) &r, sizeof(r));
    }
}

template<>
inline void
dataStore(std::ostream & stream, ColumnMajorMatrix & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((char *) &r, sizeof(r));
    }
}

template<>
inline void
dataStore(std::ostream & stream, RealTensorValue & v, void * /*context*/)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
      stream.write((char *) &v(i, j), sizeof(v(i, j)));
}

template<>
inline void
dataStore(std::ostream & stream, RealVectorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    stream.write((char *) &v(i), sizeof(v(i)));
}


// global load functions

template<typename T>
void dataLoad(std::istream & stream, T & v, void * /*context*/)
{
  // std::cout<<"Generic dataLoad"<<std::endl;

  stream.read((char *) &v, sizeof(v));
}

template<typename T>
void dataLoad(std::istream & /*stream*/, T * & /*v*/, void * /*context*/)
{
  mooseError("Cannot load raw pointers as restartable data!\nWrite a custom dataLoad() template specialization!\n\n");
}

template<>
inline void
dataLoad(std::istream & stream, Real & v, void * /*context*/)
{
  // std::cout<<"Real dataLoad"<<std::endl;

  stream.read((char *) &v, sizeof(v));
}

template<typename T>
inline void
dataLoad(std::istream & stream, std::vector<T> & v, void * context)
{
  // std::cout<<"Vector dataLoad"<<std::endl;

  // First read the size of the vector
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  v.resize(size);

  for (unsigned int i = 0; i < size; i++)
    loadHelper(stream, v[i], context);
}

template<typename T>
inline void
dataLoad(std::istream & stream, std::set<T> & s, void * context)
{
  // std::cout<<"Set dataLoad"<<std::endl;

  // First read the size of the set
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    loadHelper(stream, data, context);
    s.insert(data);
  }
}

template<typename T, typename U>
inline void
dataLoad(std::istream & stream, std::map<T,U> & m, void * context)
{
  // std::cout<<"Map dataLoad"<<std::endl;

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U data;
    loadHelper(stream, data, context);

    m[key] = data;
  }
}

template<>
inline void
dataLoad(std::istream & stream, DenseMatrix<Real> & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = 0;
      stream.read((char *) &r, sizeof(r));
      v(i, j) = r;
    }
}

template<>
inline void
dataLoad(std::istream & stream, ColumnMajorMatrix & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = 0;
      stream.read((char *) &r, sizeof(r));
      v(i, j) = r;
    }
}

template<>
inline void
dataLoad(std::istream & stream, RealTensorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
    {
      Real r = 0;
      stream.read((char *) &r, sizeof(r));
      v(i, j) = r;
    }
}

template<>
inline void
dataLoad(std::istream & stream, RealVectorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    Real r = 0;
    stream.read((char *) &r, sizeof(r));
    v(i) = r;
  }
}

// Scalar Helper Function
template<typename P>
void storeHelper(std::ostream & stream, P & data, void * context)
{
  // std::cout<<"Scalar storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// Vector Helper Function
template<typename P>
void storeHelper(std::ostream & stream, std::vector<P> & data, void * context)
{
  // std::cout<<"Vector storeHelper"<<std::endl;
  dataStore<P>(stream, data, context);
}

// Set Helper Function
template<typename P>
void storeHelper(std::ostream & stream, std::set<P> & data, void * context)
{
  // std::cout<<"Set storeHelper"<<std::endl;
  dataStore<P>(stream, data, context);
}

// Map Helper Function
template<typename P, typename Q>
void storeHelper(std::ostream & stream, std::map<P,Q> & data, void * context)
{
  // std::cout<<"Map storeHelper"<<std::endl;
  dataStore<P>(stream, data, context);
}

// Scalar Helper Function
template<typename P>
void loadHelper(std::istream & stream, P & data, void * context)
{
  // std::cout<<"Scalar loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// Vector Helper Function
template<typename P>
void loadHelper(std::istream & stream, std::vector<P> & data, void * context)
{
  // std::cout<<"Vector loadHelper"<<std::endl;
  dataLoad<P>(stream, data, context);
}

// Set Helper Function
template<typename P>
void loadHelper(std::istream & stream, std::set<P> & data, void * context)
{
  // std::cout<<"Set loadHelper"<<std::endl;
  dataLoad<P>(stream, data, context);
}

// Map Helper Function
template<typename P, typename Q>
void loadHelper(std::istream & stream, std::map<P,Q> & data, void * context)
{
  // std::cout<<"Map loadHelper"<<std::endl;
  dataLoad<P>(stream, data, context);
}

#endif /* DATAIO_H */
