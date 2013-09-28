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

#ifndef MATERIALPROPERTYIO_H
#define MATERIALPROPERTYIO_H

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
class MaterialPropertyStorage;

/**
 * This class saves stateful material properties into a file.
 *
 */
class MaterialPropertyIO
{
public:
  MaterialPropertyIO(FEProblem & fe_problem);
  virtual ~MaterialPropertyIO();

  virtual void write(const std::string & file_name);
  virtual void read(const std::string & file_name);

protected:
  FEProblem & _fe_problem;
  MooseMesh & _mesh;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;

  static const unsigned int file_version;
};

// global store functions

template<typename T>
void materialPropertyStore(std::ostream & stream, const T & v)
{
  stream.write((const char *) &v, sizeof(v));
}

template<>
inline
void materialPropertyStore(std::ostream & stream, const Real & v)
{
  stream.write((char *) &v, sizeof(v));
//  std::cout<<"value: "<<v<<std::endl;
}

template<typename T>
inline void
materialPropertyStore(std::ostream & stream, const std::vector<T> & v)
{
  for (unsigned int i = 0; i < v.size(); i++)
    materialPropertyStore(stream, v[i]);
}

template<>
inline void
materialPropertyStore(std::ostream & stream, const DenseMatrix<Real> & v)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((const char *) &r, sizeof(r));
    }
}

template<>
inline void
materialPropertyStore(std::ostream & stream, const ColumnMajorMatrix & v)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((const char *) &r, sizeof(r));
    }
}

template<>
inline void
materialPropertyStore(std::ostream & stream, const RealTensorValue & v)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
      stream.write((const char *) &v(i, j), sizeof(v(i, j)));
}

template<>
inline void
materialPropertyStore(std::ostream & stream, const RealVectorValue & v)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    stream.write((const char *) &v(i), sizeof(v(i)));
}


// global load functions

template<typename T>
void materialPropertyLoad(std::istream & stream, T & v)
{
  stream.read((char *) &v, sizeof(v));
}

template<>
inline void
materialPropertyLoad(std::istream & stream, Real & v)
{
  stream.read((char *) &v, sizeof(v));
//  std::cout<<"value: "<<v<<std::endl;
}

template<typename T>
inline void
materialPropertyLoad(std::istream & stream, std::vector<T> & v)
{
  for (unsigned int i = 0; i < v.size(); i++)
    materialPropertyLoad(stream, v[i]);
}

template<>
inline void
materialPropertyLoad(std::istream & stream, DenseMatrix<Real> & v)
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
materialPropertyLoad(std::istream & stream, ColumnMajorMatrix & v)
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
materialPropertyLoad(std::istream & stream, RealTensorValue & v)
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
materialPropertyLoad(std::istream & stream, RealVectorValue & v)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    Real r = 0;
    stream.read((char *) &r, sizeof(r));
    v(i) = r;
  }
}

#endif /* MATERIALPROPERTYIO_H */
