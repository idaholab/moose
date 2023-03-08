//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DenseMatrix.h"
#include "MooseConfig.h"
#include "DataIO.h"
#include "MooseMesh.h"

#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"

#include "DualRealOps.h"

template <>
void
dataStore(std::ostream & stream, Real & v, void * /*context*/)
{
  stream.write((char *)&v, sizeof(v));
}

template <>
void
dataStore(std::ostream & stream, std::string & v, void * /*context*/)
{
  // Write the size of the string
  unsigned int size = v.size();
  stream.write((char *)&size, sizeof(size));

  // Write the string (Do not store the null byte)
  stream.write(v.c_str(), sizeof(char) * size);
}

template <>
void
dataStore(std::ostream & stream, bool & v, void * /*context*/)
{
  stream.write((char *)&v, sizeof(v));
}

template <>
void
dataStore(std::ostream & stream, std::vector<bool> & v, void * context)
{
  for (bool b : v)
    dataStore(stream, b, context);
}

template <>
void
dataStore(std::ostream & stream, RankTwoTensor & rtt, void * context)
{
  dataStore(stream, rtt._coords, context);
}

template <>
void
dataStore(std::ostream & stream, RankThreeTensor & rtht, void * context)
{
  dataStore(stream, rtht._vals, context);
}

template <>
void
dataStore(std::ostream & stream, RankFourTensor & rft, void * context)
{
  dataStore(stream, rft._vals, context);
}

template <>
void
dataStore(std::ostream & stream, DualReal & dn, void * context)
{
  dataStore(stream, dn.value(), context);

  if (DualReal::do_derivatives)
  {
    auto & derivatives = dn.derivatives();
    std::size_t size = derivatives.size();
    dataStore(stream, size, context);
    for (MooseIndex(size) i = 0; i < size; ++i)
    {
      dataStore(stream, derivatives.raw_index(i), context);
      dataStore(stream, derivatives.raw_at(i), context);
    }
  }
}

template <>
void
dataStore(std::ostream & stream, const Elem *& e, void * context)
{
  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (e)
  {
    id = e->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }

  storeHelper(stream, id, context);
}

template <>
void
dataStore(std::ostream & stream, const Node *& n, void * context)
{
  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (n)
  {
    id = n->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }

  storeHelper(stream, id, context);
}

template <>
void
dataStore(std::ostream & stream, Elem *& e, void * context)
{
  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (e)
  {
    id = e->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }

  storeHelper(stream, id, context);
}

template <>
void
dataStore(std::ostream & stream, Node *& n, void * context)
{
  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (n)
  {
    id = n->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }

  storeHelper(stream, id, context);
}

template <>
void
dataStore(std::ostream & stream, std::stringstream & s, void * /* context */)
{
  const std::string & s_str = s.str();

  size_t s_size = s_str.size();
  stream.write((char *)&s_size, sizeof(s_size));

  stream.write(s_str.c_str(), sizeof(char) * (s_str.size()));
}

template <>
void
dataStore(std::ostream & stream, std::stringstream *& s, void * context)
{
  dataStore(stream, *s, context);
}

template <>
void
dataStore(std::ostream & stream, RealEigenVector & v, void * context)
{
  unsigned int m = v.size();
  stream.write((char *)&m, sizeof(m));
  for (unsigned int i = 0; i < v.size(); i++)
  {
    Real r = v(i);
    dataStore(stream, r, context);
  }
}

template <>
void
dataStore(std::ostream & stream, RealEigenMatrix & v, void * context)
{
  unsigned int m = v.rows();
  stream.write((char *)&m, sizeof(m));
  unsigned int n = v.cols();
  stream.write((char *)&n, sizeof(n));
  for (unsigned int i = 0; i < m; i++)
    for (unsigned int j = 0; j < n; j++)
    {
      Real r = v(i, j);
      dataStore(stream, r, context);
    }
}

template <typename T>
void
dataStore(std::ostream & stream, TensorValue<T> & v, void * context)
{
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
    {
      T r = v(i, j);
      dataStore(stream, r, context);
    }
}

template void dataStore(std::ostream & stream, TensorValue<Real> & v, void * context);
template void dataStore(std::ostream & stream, TensorValue<DualReal> & v, void * context);

template <typename T>
void
dataStore(std::ostream & stream, DenseMatrix<T> & v, void * context)
{
  unsigned int m = v.m();
  unsigned int n = v.n();
  stream.write((char *)&m, sizeof(m));
  stream.write((char *)&n, sizeof(n));
  for (unsigned int i = 0; i < m; i++)
    for (unsigned int j = 0; j < n; j++)
    {
      T r = v(i, j);
      dataStore(stream, r, context);
    }
}

template void dataStore(std::ostream & stream, DenseMatrix<Real> & v, void * context);
template void dataStore(std::ostream & stream, DenseMatrix<DualReal> & v, void * context);

template <typename T>
void
dataStore(std::ostream & stream, VectorValue<T> & v, void * context)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (const auto i : make_range(Moose::dim))
  {
    T r = v(i);
    dataStore(stream, r, context);
  }
}

template void dataStore(std::ostream & stream, VectorValue<Real> & v, void * context);
template void dataStore(std::ostream & stream, VectorValue<DualReal> & v, void * context);

void
dataStore(std::ostream & stream, Point & p, void * context)
{
  for (const auto i : make_range(Moose::dim))
  {
    Real r = p(i);
    dataStore(stream, r, context);
  }
}

template <>
void
dataStore(std::ostream & stream, libMesh::Parameters & p, void * context)
{
  // First store the size of the map
  unsigned int size = p.n_parameters();
  stream.write((char *)&size, sizeof(size));

  auto it = p.begin();
  auto end = p.end();

  for (; it != end; ++it)
  {
    auto & key = const_cast<std::string &>(it->first);
    auto type = it->second->type();

    storeHelper(stream, key, context);
    storeHelper(stream, type, context);

#define storescalar(ptype)                                                                         \
  else if (it->second->type() == demangle(typeid(ptype).name())) storeHelper(                      \
      stream,                                                                                      \
      (dynamic_cast<libMesh::Parameters::Parameter<ptype> *>(MooseUtils::get(it->second)))->get(), \
      context)

    if (false)
      ;
    storescalar(Real);
    storescalar(short);
    storescalar(int);
    storescalar(long);
    storescalar(unsigned short);
    storescalar(unsigned int);
    storescalar(unsigned long);

#undef storescalar
  }
}

// global load functions

template <>
void
dataLoad(std::istream & stream, Real & v, void * /*context*/)
{
  stream.read((char *)&v, sizeof(v));
}

template <>
void
dataLoad(std::istream & stream, std::string & v, void * /*context*/)
{
  // Read the size of the string
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  // Resize the string data
  v.resize(size);

  // Read the string
  stream.read(&v[0], sizeof(char) * size);
}

template <>
void
dataLoad(std::istream & stream, bool & v, void * /*context*/)
{
  stream.read((char *)&v, sizeof(v));
}

template <>
void
dataLoad(std::istream & stream, std::vector<bool> & v, void * context)
{
  for (bool b : v)
    dataLoad(stream, b, context);
}

template <>
void
dataLoad(std::istream & stream, DualReal & dn, void * context)
{
  dataLoad(stream, dn.value(), context);

  if (DualReal::do_derivatives)
  {
    auto & derivatives = dn.derivatives();
    std::size_t size = 0;
    stream.read((char *)&size, sizeof(size));
    derivatives.resize(size);

    for (MooseIndex(derivatives) i = 0; i < derivatives.size(); ++i)
    {
      dataLoad(stream, derivatives.raw_index(i), context);
      dataLoad(stream, derivatives.raw_at(i), context);
    }
  }
}

template <>
void
dataLoad(std::istream & stream, const Elem *& e, void * context)
{
  if (!context)
    mooseError("Can only load Elem objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // TODO: Write out the unique ID of this element
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, context);

  if (id != libMesh::DofObject::invalid_id)
    e = mesh->elemPtr(id);
  else
    e = NULL;
}

template <>
void
dataLoad(std::istream & stream, const Node *& n, void * context)
{
  if (!context)
    mooseError("Can only load Node objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, context);

  if (id != libMesh::DofObject::invalid_id)
    n = mesh->nodePtr(id);
  else
    n = NULL;
}

template <>
void
dataLoad(std::istream & stream, Elem *& e, void * context)
{
  if (!context)
    mooseError("Can only load Elem objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // TODO: Write out the unique ID of this element
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, context);

  if (id != libMesh::DofObject::invalid_id)
    e = mesh->elemPtr(id);
  else
    e = NULL;
}

template <>
void
dataLoad(std::istream & stream, Node *& n, void * context)
{
  if (!context)
    mooseError("Can only load Node objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, context);

  if (id != libMesh::DofObject::invalid_id)
    n = mesh->nodePtr(id);
  else
    n = NULL;
}

template <>
void
dataLoad(std::istream & stream, std::stringstream & s, void * /* context */)
{
  size_t s_size = 0;
  stream.read((char *)&s_size, sizeof(s_size));

  std::unique_ptr<char[]> s_s = std::make_unique<char[]>(s_size);
  stream.read(s_s.get(), s_size);

  // Clear the stringstream before loading new data into it.
  s.str(std::string());
  s.write(s_s.get(), s_size);
}

template <>
void
dataLoad(std::istream & stream, std::stringstream *& s, void * context)
{
  dataLoad(stream, *s, context);
}

template <>
void
dataLoad(std::istream & stream, RealEigenVector & v, void * context)
{
  unsigned int n = 0;
  stream.read((char *)&n, sizeof(n));
  v.resize(n);
  for (unsigned int i = 0; i < n; i++)
  {
    Real r = 0;
    dataLoad(stream, r, context);
    v(i) = r;
  }
}

template <>
void
dataLoad(std::istream & stream, RealEigenMatrix & v, void * context)
{
  unsigned int m = 0;
  stream.read((char *)&m, sizeof(m));
  unsigned int n = 0;
  stream.read((char *)&n, sizeof(n));
  v.resize(m, n);
  for (unsigned int i = 0; i < m; i++)
    for (unsigned int j = 0; j < n; j++)
    {
      Real r = 0;
      dataLoad(stream, r, context);
      v(i, j) = r;
    }
}

template <typename T>
void
dataLoad(std::istream & stream, TensorValue<T> & v, void * context)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
    {
      T r = 0;
      dataLoad(stream, r, context);
      v(i, j) = r;
    }
}

template void dataLoad(std::istream & stream, TensorValue<Real> & v, void * context);
template void dataLoad(std::istream & stream, TensorValue<DualReal> & v, void * context);

template <typename T>
void
dataLoad(std::istream & stream, DenseMatrix<T> & v, void * context)
{
  unsigned int m = 0, n = 0;
  stream.read((char *)&m, sizeof(m));
  stream.read((char *)&n, sizeof(n));
  v.resize(m, n);
  for (unsigned int i = 0; i < m; i++)
    for (unsigned int j = 0; j < n; j++)
    {
      T r = 0;
      dataLoad(stream, r, context);
      v(i, j) = r;
    }
}

template void dataLoad(std::istream & stream, DenseMatrix<Real> & v, void * context);
template void dataLoad(std::istream & stream, DenseMatrix<DualReal> & v, void * context);

template <typename T>
void
dataLoad(std::istream & stream, VectorValue<T> & v, void * context)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (const auto i : make_range(Moose::dim))
  {
    T r = 0;
    dataLoad(stream, r, context);
    v(i) = r;
  }
}

template void dataLoad(std::istream & stream, VectorValue<Real> & v, void * context);
template void dataLoad(std::istream & stream, VectorValue<DualReal> & v, void * context);

void
dataLoad(std::istream & stream, Point & p, void * context)
{
  for (const auto i : make_range(Moose::dim))
  {
    Real r = 0;
    dataLoad(stream, r, context);
    p(i) = r;
  }
}

template <>
void
dataLoad(std::istream & stream, libMesh::Parameters & p, void * context)
{
  p.clear();

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  for (unsigned int i = 0; i < size; i++)
  {
    std::string key, type;
    loadHelper(stream, key, context);
    loadHelper(stream, type, context);

#define loadscalar(ptype)                                                                          \
  else if (type == demangle(typeid(ptype).name())) do                                              \
  {                                                                                                \
    ptype & value = p.set<ptype>(key);                                                             \
    loadHelper(stream, value, context);                                                            \
  }                                                                                                \
  while (0)

    if (false)
      ;
    loadscalar(Real);
    loadscalar(short);
    loadscalar(int);
    loadscalar(long);
    loadscalar(unsigned short);
    loadscalar(unsigned int);
    loadscalar(unsigned long);

#undef loadscalar
  }
}

template <>
void
dataLoad(std::istream & stream, Vec & v, void * context)
{
  PetscInt local_size;
  VecGetLocalSize(v, &local_size);
  PetscScalar * array;
  VecGetArray(v, &array);
  for (PetscInt i = 0; i < local_size; i++)
    dataLoad(stream, array[i], context);

  VecRestoreArray(v, &array);
}

template <>
void
dataStore(std::ostream & stream, Vec & v, void * context)
{
  PetscInt local_size;
  VecGetLocalSize(v, &local_size);
  PetscScalar * array;
  VecGetArray(v, &array);
  for (PetscInt i = 0; i < local_size; i++)
    dataStore(stream, array[i], context);

  VecRestoreArray(v, &array);
}
