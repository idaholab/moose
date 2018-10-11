#ifndef DUALNUMBERTYPETENSOR_H
#define DUALNUMBERTYPETENSOR_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_METAPHYSICL

#include "libmesh/type_tensor.h"
#include "metaphysicl/compare_types.h"

namespace MetaPhysicL
{

template <typename T, bool reverseorder>
struct PlusType<libMesh::TypeTensor<T>, libMesh::TypeTensor<T>, reverseorder>
{
  typedef libMesh::TypeTensor<T> supertype;
};

template <typename T, bool reverseorder>
struct MinusType<libMesh::TypeTensor<T>, libMesh::TypeTensor<T>, reverseorder>
{
  typedef libMesh::TypeTensor<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<libMesh::TypeTensor<T>, libMesh::TypeTensor<T>, reverseorder>
{
  typedef libMesh::TypeTensor<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<Real, libMesh::TypeTensor<T>, reverseorder>
{
  typedef libMesh::TypeTensor<T> supertype;
};

template <typename T>
struct MultipliesType<libMesh::TypeTensor<T>, libMesh::TypeVector<T>>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T>
struct DividesType<libMesh::TypeTensor<T>, Real>
{
  typedef libMesh::TypeTensor<T> supertype;
};
}

#endif
#endif
