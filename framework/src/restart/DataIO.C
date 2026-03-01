//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DenseMatrix.h"
#include "DataIO.h"
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/enum_solver_package.h"
#include "libmesh/petsc_solver_exception.h"

using namespace libMesh;

template <typename Context>
void
dataStore(std::ostream & stream, Real & v, Context /*context*/)
{
  stream.write((char *)&v, sizeof(v));
}

template void dataStore(std::ostream & stream, Real & v, void * context);
template void dataStore(std::ostream & stream, Real & v, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, std::string & v, Context /*context*/)
{
  // Write the size of the string
  unsigned int size = v.size();
  stream.write((char *)&size, sizeof(size));

  // Write the string (Do not store the null byte)
  stream.write(v.c_str(), sizeof(char) * size);
}

template void dataStore(std::ostream & stream, std::string & v, void * context);
template void dataStore(std::ostream & stream, std::string & v, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, VariableName & v, Context context)
{
  auto & name = static_cast<std::string &>(v);
  dataStore(stream, name, context);
}

template void dataStore(std::ostream & stream, VariableName & v, void * context);
template void dataStore(std::ostream & stream, VariableName & v, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, UserObjectName & v, Context context)
{
  auto & name = static_cast<std::string &>(v);
  dataStore(stream, name, context);
}

template void dataStore(std::ostream & stream, UserObjectName & v, void * context);
template void dataStore(std::ostream & stream, UserObjectName & v, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, bool & v, Context /*context*/)
{
  stream.write((char *)&v, sizeof(v));
}

template void dataStore(std::ostream & stream, bool & v, void * context);
template void dataStore(std::ostream & stream, bool & v, std::nullptr_t context);

// std::vector<bool> is defined inline in DataIO.h (uses proxy references)

template <typename Context>
void
dataStore(std::ostream & stream, ADReal & dn, Context context)
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

template void dataStore(std::ostream & stream, ADReal & dn, void * context);
template void dataStore(std::ostream & stream, ADReal & dn, std::nullptr_t context);

// Elem/Node store: accepts any context (just stores ID, ignores context)
template <typename Context>
void
dataStore(std::ostream & stream, const Elem *& e, Context /*context*/)
{
  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (e)
  {
    id = e->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }

  storeHelper(stream, id, nullptr);
}

template void dataStore(std::ostream & stream, const Elem *& e, void * context);
template void dataStore(std::ostream & stream, const Elem *& e, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, const Node *& n, Context /*context*/)
{
  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (n)
  {
    id = n->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }

  storeHelper(stream, id, nullptr);
}

template void dataStore(std::ostream & stream, const Node *& n, void * context);
template void dataStore(std::ostream & stream, const Node *& n, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, Elem *& e, Context /*context*/)
{
  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (e)
  {
    id = e->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }

  storeHelper(stream, id, nullptr);
}

template void dataStore(std::ostream & stream, Elem *& e, void * context);
template void dataStore(std::ostream & stream, Elem *& e, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, Node *& n, Context /*context*/)
{
  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if (n)
  {
    id = n->id();
    if (id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }

  storeHelper(stream, id, nullptr);
}

template void dataStore(std::ostream & stream, Node *& n, void * context);
template void dataStore(std::ostream & stream, Node *& n, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, std::stringstream & s, Context /* context */)
{
  const std::string & s_str = s.str();

  size_t s_size = s_str.size();
  stream.write((char *)&s_size, sizeof(s_size));

  stream.write(s_str.c_str(), sizeof(char) * (s_str.size()));
}

template void dataStore(std::ostream & stream, std::stringstream & s, void * context);
template void dataStore(std::ostream & stream, std::stringstream & s, std::nullptr_t context);

template <typename T, typename Context>
void
dataStore(std::ostream & stream, TensorValue<T> & v, Context context)
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

template <typename T, typename Context>
void
dataStore(std::ostream & stream, DenseMatrix<T> & v, Context context)
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

template <typename T, typename Context>
void
dataStore(std::ostream & stream, VectorValue<T> & v, Context context)
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

template <typename Context>
void
dataStore(std::ostream & stream, Point & p, Context context)
{
  for (const auto i : make_range(Moose::dim))
  {
    Real r = p(i);
    dataStore(stream, r, context);
  }
}

template void dataStore(std::ostream & stream, Point & p, void * context);

template <typename Context>
void
dataStore(std::ostream & stream, libMesh::Parameters & p, Context context)
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

template void dataStore(std::ostream & stream, libMesh::Parameters & p, void * context);
template void dataStore(std::ostream & stream, libMesh::Parameters & p, std::nullptr_t context);

void
dataStore(std::ostream & stream,
          std::unique_ptr<libMesh::NumericVector<Number>> & v,
          const libMesh::Parallel::Communicator & context)
{
  // Classes may declare unique pointers to vectors as restartable data and never actually create
  // vector instances. This happens for example in the `TimeIntegrator` class where subvector
  // instances are only created if multiple time integrators are present
  bool have_vector = v.get();
  dataStore(stream, have_vector, nullptr);
  if (!have_vector)
    return;

  const auto & comm = context;
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

template <typename Context>
void
dataLoad(std::istream & stream, Real & v, Context /*context*/)
{
  stream.read((char *)&v, sizeof(v));
}

template void dataLoad(std::istream & stream, Real & v, void * context);
template void dataLoad(std::istream & stream, Real & v, std::nullptr_t context);

template <typename Context>
void
dataLoad(std::istream & stream, std::string & v, Context /*context*/)
{
  // Read the size of the string
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  // Resize the string data
  v.resize(size);

  // Read the string
  stream.read(&v[0], sizeof(char) * size);
}

template void dataLoad(std::istream & stream, std::string & v, void * context);
template void dataLoad(std::istream & stream, std::string & v, std::nullptr_t context);

template <typename Context>
void
dataLoad(std::istream & stream, VariableName & v, Context context)
{
  auto & name = static_cast<std::string &>(v);
  dataLoad(stream, name, context);
}

template void dataLoad(std::istream & stream, VariableName & v, void * context);
template void dataLoad(std::istream & stream, VariableName & v, std::nullptr_t context);

template <typename Context>
void
dataLoad(std::istream & stream, UserObjectName & v, Context context)
{
  auto & name = static_cast<std::string &>(v);
  dataLoad(stream, name, context);
}

template void dataLoad(std::istream & stream, UserObjectName & v, void * context);
template void dataLoad(std::istream & stream, UserObjectName & v, std::nullptr_t context);

template <typename Context>
void
dataLoad(std::istream & stream, bool & v, Context /*context*/)
{
  stream.read((char *)&v, sizeof(v));
}

template void dataLoad(std::istream & stream, bool & v, void * context);
template void dataLoad(std::istream & stream, bool & v, std::nullptr_t context);

// std::vector<bool> is defined inline in DataIO.h (uses proxy references)

template <typename Context>
void
dataLoad(std::istream & stream, ADReal & dn, Context context)
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

template void dataLoad(std::istream & stream, ADReal & dn, void * context);
template void dataLoad(std::istream & stream, ADReal & dn, std::nullptr_t context);

// Elem/Node load: REQUIRES MeshBase* context (non-templated, specific context type required)
void
dataLoad(std::istream & stream, const Elem *& e, libMesh::MeshBase * mesh)
{
  mooseAssert(mesh, "Can only load Elem objects using a mesh context!");

  // TODO: Write out the unique ID of this element
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, nullptr);

  if (id != libMesh::DofObject::invalid_id)
    e = mesh->elem_ptr(id);
  else
    e = nullptr;
}

void
dataLoad(std::istream & stream, const Node *& n, libMesh::MeshBase * mesh)
{
  mooseAssert(mesh, "Can only load Node objects using a mesh context!");

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, nullptr);

  if (id != libMesh::DofObject::invalid_id)
    n = mesh->node_ptr(id);
  else
    n = nullptr;
}

void
dataLoad(std::istream & stream, Elem *& e, libMesh::MeshBase * mesh)
{
  mooseAssert(mesh, "Can only load Elem objects using a mesh context!");

  // TODO: Write out the unique ID of this element
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, nullptr);

  if (id != libMesh::DofObject::invalid_id)
    e = mesh->elem_ptr(id);
  else
    e = nullptr;
}

void
dataLoad(std::istream & stream, Node *& n, libMesh::MeshBase * mesh)
{
  mooseAssert(mesh, "Can only load Node objects using a mesh context!");

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id = libMesh::DofObject::invalid_id;

  loadHelper(stream, id, nullptr);

  if (id != libMesh::DofObject::invalid_id)
    n = mesh->node_ptr(id);
  else
    n = NULL;
}

template <typename Context>
void
dataLoad(std::istream & stream, std::stringstream & s, Context /* context */)
{
  size_t s_size = 0;
  stream.read((char *)&s_size, sizeof(s_size));

  std::unique_ptr<char[]> s_s = std::make_unique<char[]>(s_size);
  stream.read(s_s.get(), s_size);

  // Clear the stringstream before loading new data into it.
  s.str(std::string());
  s.write(s_s.get(), s_size);
}

template void dataLoad(std::istream & stream, std::stringstream & s, void * context);
template void dataLoad(std::istream & stream, std::stringstream & s, std::nullptr_t context);

template <typename T, typename Context>
void
dataLoad(std::istream & stream, TensorValue<T> & v, Context context)
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

template <typename T, typename Context>
void
dataLoad(std::istream & stream, DenseMatrix<T> & v, Context context)
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

template <typename T, typename Context>
void
dataLoad(std::istream & stream, VectorValue<T> & v, Context context)
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

template <typename Context>
void
dataLoad(std::istream & stream, Point & p, Context context)
{
  for (const auto i : make_range(Moose::dim))
  {
    Real r = 0;
    dataLoad(stream, r, context);
    p(i) = r;
  }
}

template void dataLoad(std::istream & stream, Point & p, void * context);

template <typename Context>
void
dataLoad(std::istream & stream, libMesh::Parameters & p, Context context)
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

template void dataLoad(std::istream & stream, libMesh::Parameters & p, void * context);
template void dataLoad(std::istream & stream, libMesh::Parameters & p, std::nullptr_t context);

void
dataLoad(std::istream & stream,
         std::unique_ptr<libMesh::NumericVector<Number>> & v,
         const libMesh::Parallel::Communicator & context)
{
  bool have_vector;
  dataLoad(stream, have_vector, nullptr);

  if (!have_vector)
    return;

  const auto & comm = context;
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

template <typename Context>
void
dataLoad(std::istream & stream, Vec & v, Context context)
{
  PetscInt local_size;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetLocalSize(v, &local_size));
  PetscScalar * array;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetArray(v, &array));
  for (PetscInt i = 0; i < local_size; i++)
    dataLoad(stream, array[i], context);

  LibmeshPetscCallA(PETSC_COMM_WORLD, VecRestoreArray(v, &array));
}

template void dataLoad(std::istream & stream, Vec & v, void * context);
template void dataLoad(std::istream & stream, Vec & v, std::nullptr_t context);

template <typename Context>
void
dataStore(std::ostream & stream, Vec & v, Context context)
{
  PetscInt local_size;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetLocalSize(v, &local_size));
  PetscScalar * array;
  LibmeshPetscCallA(PETSC_COMM_WORLD, VecGetArray(v, &array));
  for (PetscInt i = 0; i < local_size; i++)
    dataStore(stream, array[i], context);

  LibmeshPetscCallA(PETSC_COMM_WORLD, VecRestoreArray(v, &array));
}

template void dataStore(std::ostream & stream, Vec & v, void * context);
template void dataStore(std::ostream & stream, Vec & v, std::nullptr_t context);

// Additional explicit template instantiations for common context types
// Basic types with std::nullptr_t
template void dataStore(std::ostream &, Point &, std::nullptr_t);
template void dataLoad(std::istream &, Point &, std::nullptr_t);
template void dataStore(std::ostream &, VectorValue<Real> &, std::nullptr_t);
template void dataLoad(std::istream &, VectorValue<Real> &, std::nullptr_t);
template void dataStore(std::ostream &, TensorValue<Real> &, std::nullptr_t);
template void dataLoad(std::istream &, TensorValue<Real> &, std::nullptr_t);
template void dataStore(std::ostream &, DenseMatrix<Real> &, std::nullptr_t);
template void dataLoad(std::istream &, DenseMatrix<Real> &, std::nullptr_t);

// ADReal types with std::nullptr_t
template void dataStore(std::ostream &, VectorValue<ADReal> &, std::nullptr_t);
template void dataLoad(std::istream &, VectorValue<ADReal> &, std::nullptr_t);
template void dataStore(std::ostream &, TensorValue<ADReal> &, std::nullptr_t);
template void dataLoad(std::istream &, TensorValue<ADReal> &, std::nullptr_t);
template void dataStore(std::ostream &, DenseMatrix<ADReal> &, std::nullptr_t);
template void dataLoad(std::istream &, DenseMatrix<ADReal> &, std::nullptr_t);

// Basic types with MeshBase*
template void dataStore(std::ostream &, Real &, libMesh::MeshBase *);
template void dataLoad(std::istream &, Real &, libMesh::MeshBase *);
template void dataStore(std::ostream &, Point &, libMesh::MeshBase *);
template void dataLoad(std::istream &, Point &, libMesh::MeshBase *);
template void dataStore(std::ostream &, VectorValue<Real> &, libMesh::MeshBase *);
template void dataLoad(std::istream &, VectorValue<Real> &, libMesh::MeshBase *);

// Elem/Node pointers with MeshBase*
template void dataStore(std::ostream &, const libMesh::Elem *&, libMesh::MeshBase *);
template void dataLoad(std::istream &, const libMesh::Elem *&, libMesh::MeshBase *);
template void dataStore(std::ostream &, const libMesh::Node *&, libMesh::MeshBase *);
template void dataLoad(std::istream &, const libMesh::Node *&, libMesh::MeshBase *);
template void dataStore(std::ostream &, libMesh::Elem *&, libMesh::MeshBase *);
template void dataLoad(std::istream &, libMesh::Elem *&, libMesh::MeshBase *);
template void dataStore(std::ostream &, libMesh::Node *&, libMesh::MeshBase *);
template void dataLoad(std::istream &, libMesh::Node *&, libMesh::MeshBase *);

// NumericVector with const Communicator*
template void dataStore(std::ostream &,
                        std::unique_ptr<libMesh::NumericVector<Number>> &,
                        const libMesh::Parallel::Communicator *);
template void dataLoad(std::istream &,
                       std::unique_ptr<libMesh::NumericVector<Number>> &,
                       const libMesh::Parallel::Communicator *);
