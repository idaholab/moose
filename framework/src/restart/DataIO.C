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
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include "libmesh/elem.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/enum_solver_package.h"
#include "libmesh/petsc_solver_exception.h"

using namespace libMesh;

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
dataStore(std::ostream & stream, VariableName & v, void * context)
{
  auto & name = static_cast<std::string &>(v);
  dataStore(stream, name, context);
}

template <>
void
dataStore(std::ostream & stream, UserObjectName & v, void * context)
{
  auto & name = static_cast<std::string &>(v);
  dataStore(stream, name, context);
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
dataStore(std::ostream & stream, ADReal & dn, void * context)
{
  dataStore(stream, dn.value(), context);

  if (ADReal::do_derivatives)
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
template void dataStore(std::ostream & stream, TensorValue<ADReal> & v, void * context);

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
template void dataStore(std::ostream & stream, DenseMatrix<ADReal> & v, void * context);

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
template void dataStore(std::ostream & stream, VectorValue<ADReal> & v, void * context);

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

template <>
void
dataStore(std::ostream & stream,
          std::unique_ptr<libMesh::NumericVector<Number>> & v,
          void * context)
{
  // Classes may declare unique pointers to vectors as restartable data and never actually create
  // vector instances. This happens for example in the `TimeIntegrator` class where subvector
  // instances are only created if multiple time integrators are present
  bool have_vector = v.get();
  dataStore(stream, have_vector, context);
  if (!have_vector)
    return;

  mooseAssert(context, "Needs a context of the communicator");
  const auto & comm = *static_cast<const libMesh::Parallel::Communicator *>(context);
  mooseAssert(&comm == &v->comm(), "Inconsistent communicator");

  if (v->type() == GHOSTED)
    mooseError("Cannot store ghosted numeric vectors");

  // Store the communicator size for sanity checking later
  unsigned int comm_size = comm.size();
  dataStore(stream, comm_size, nullptr);

  // Store the solver package so that we know what vector type to construct
  libMesh::SolverPackage solver_package;
  if (dynamic_cast<libMesh::PetscVector<Number> *>(v.get()))
    solver_package = PETSC_SOLVERS;
  else
    mooseError("Can only store unique_ptrs of PetscVectors");
  int solver_package_int = solver_package;
  dataStore(stream, solver_package_int, nullptr);

  // Store the sizes
  dof_id_type size = v->size();
  dataStore(stream, size, nullptr);
  dof_id_type local_size = v->local_size();
  dataStore(stream, local_size, nullptr);

  // Store the vector itself
  dataStore(stream, *v, nullptr);
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
dataLoad(std::istream & stream, VariableName & v, void * context)
{
  auto & name = static_cast<std::string &>(v);
  dataLoad(stream, name, context);
}

template <>
void
dataLoad(std::istream & stream, UserObjectName & v, void * context)
{
  auto & name = static_cast<std::string &>(v);
  dataLoad(stream, name, context);
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
dataLoad(std::istream & stream, ADReal & dn, void * context)
{
  dataLoad(stream, dn.value(), context);

  if (ADReal::do_derivatives)
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
template void dataLoad(std::istream & stream, TensorValue<ADReal> & v, void * context);

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
template void dataLoad(std::istream & stream, DenseMatrix<ADReal> & v, void * context);

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
template void dataLoad(std::istream & stream, VectorValue<ADReal> & v, void * context);

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
dataLoad(std::istream & stream, std::unique_ptr<libMesh::NumericVector<Number>> & v, void * context)
{
  bool have_vector;
  dataLoad(stream, have_vector, context);

  if (!have_vector)
    return;

  mooseAssert(context, "Needs a context of the communicator");
  const auto & comm = *static_cast<const libMesh::Parallel::Communicator *>(context);
  if (v)
    mooseAssert(&comm == &v->comm(), "Inconsistent communicator");

  // Load the communicator size for consistency checks
  unsigned int comm_size;
  dataLoad(stream, comm_size, nullptr);
  mooseAssert(comm.size() == comm_size, "Inconsistent communicator size");

  // Load the solver package to build the vector
  int solver_package_int;
  dataLoad(stream, solver_package_int, nullptr);
  libMesh::SolverPackage solver_package = static_cast<libMesh::SolverPackage>(solver_package_int);

  // Load the sizes
  dof_id_type size, local_size;
  dataLoad(stream, size, nullptr);
  dataLoad(stream, local_size, nullptr);

  // Construct the vector given the type, only if we need to. v could be non-null here
  // if we're advancing back and loading a backup
  if (!v)
  {
    v = NumericVector<Number>::build(comm, solver_package);
    v->init(size, local_size);
  }
  else
    mooseAssert(v->type() != GHOSTED, "Cannot be ghosted");

  // Make sure that the sizes are consistent; this will happen if we're calling this
  // on a vector that has already been loaded previously
  mooseAssert(v->size() == size, "Inconsistent size");
  mooseAssert(v->local_size() == local_size, "Inconsistent local size");

  // Now that we have an initialized vector, fill the entries
  dataLoad(stream, *v, nullptr);
}

template <>
void
dataLoad(std::istream & stream, Vec & v, void * context)
{
  PetscInt local_size;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetLocalSize(v, &local_size));
  PetscScalar * array;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetArray(v, &array));
  for (PetscInt i = 0; i < local_size; i++)
    dataLoad(stream, array[i], context);

  LibmeshPetscCallA(PETSC_COMM_WORLD, VecRestoreArray(v, &array));
}

template <>
void
dataStore(std::ostream & stream, Vec & v, void * context)
{
  PetscInt local_size;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetLocalSize(v, &local_size));
  PetscScalar * array;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetArray(v, &array));
  for (PetscInt i = 0; i < local_size; i++)
    dataStore(stream, array[i], context);

  LibmeshPetscCallA(PETSC_COMM_WORLD, VecRestoreArray(v, &array));
}
