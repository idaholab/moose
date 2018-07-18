#ifndef DUALNUMBERTYPEVECTOR_H
#define DUALNUMBERTYPEVECTOR_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_METAPHYSICL

#include "libmesh/type_vector.h"
#include "metaphysicl/compare_types.h"

namespace MetaPhysicL
{

template <typename T, bool reverseorder>
struct PlusType<libMesh::TypeVector<T>, libMesh::TypeVector<T>, reverseorder>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T, bool reverseorder>
struct MinusType<libMesh::TypeVector<T>, libMesh::TypeVector<T>, reverseorder>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<libMesh::TypeVector<T>, libMesh::TypeVector<T>, reverseorder>
{
  typedef Real supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<Real, libMesh::TypeVector<T>, reverseorder>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T>
struct DividesType<libMesh::TypeVector<T>, Real>
{
  typedef libMesh::TypeVector<T> supertype;
};
}

#endif
#endif
