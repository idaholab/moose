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

#include "DataIO.h"
#include "MooseMesh.h"
#include "ColumnMajorMatrix.h"

// libMesh includes
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/elem.h"

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
dataStore(std::ostream & stream, NumericVector<Real> & v, void * /*context*/)
{
  v.close();

  numeric_index_type size = v.local_size();

  for (numeric_index_type i = v.first_local_index(); i < v.first_local_index() + size; i++)
  {
    Real r = v(i);
    stream.write((char *)&r, sizeof(r));
  }
}

template <>
void
dataStore(std::ostream & stream, DenseVector<Real> & v, void * /*context*/)
{
  unsigned int m = v.size();
  stream.write((char *)&m, sizeof(m));
  for (unsigned int i = 0; i < v.size(); i++)
  {
    Real r = v(i);
    stream.write((char *)&r, sizeof(r));
  }
}

template <>
void
dataStore(std::ostream & stream, DenseMatrix<Real> & v, void * /*context*/)
{
  unsigned int m = v.m();
  unsigned int n = v.n();
  stream.write((char *)&m, sizeof(m));
  stream.write((char *)&n, sizeof(n));
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((char *)&r, sizeof(r));
    }
}

template <>
void
dataStore(std::ostream & stream, ColumnMajorMatrix & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = v(i, j);
      stream.write((char *)&r, sizeof(r));
    }
}

template <>
void
dataStore(std::ostream & stream, RealTensorValue & v, void * /*context*/)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
      stream.write((char *)&v(i, j), sizeof(v(i, j)));
}

template <>
void
dataStore(std::ostream & stream, RealVectorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    stream.write((char *)&v(i), sizeof(v(i)));
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
dataLoad(std::istream & stream, NumericVector<Real> & v, void * /*context*/)
{
  numeric_index_type size = v.local_size();

  for (numeric_index_type i = v.first_local_index(); i < v.first_local_index() + size; i++)
  {
    Real r = 0;
    stream.read((char *)&r, sizeof(r));
    v.set(i, r);
  }

  v.close();
}

template <>
void
dataLoad(std::istream & stream, DenseVector<Real> & v, void * /*context*/)
{
  unsigned int n = 0;
  stream.read((char *)&n, sizeof(n));
  v.resize(n);
  for (unsigned int i = 0; i < n; i++)
  {
    Real r = 0;
    stream.read((char *)&r, sizeof(r));
    v(i) = r;
  }
}

template <>
void
dataLoad(std::istream & stream, DenseMatrix<Real> & v, void * /*context*/)
{
  unsigned int nr = 0, nc = 0;
  stream.read((char *)&nr, sizeof(nr));
  stream.read((char *)&nc, sizeof(nc));
  v.resize(nr, nc);
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = 0;
      stream.read((char *)&r, sizeof(r));
      v(i, j) = r;
    }
}

template <>
void
dataLoad(std::istream & stream, ColumnMajorMatrix & v, void * /*context*/)
{
  for (unsigned int i = 0; i < v.m(); i++)
    for (unsigned int j = 0; j < v.n(); j++)
    {
      Real r = 0;
      stream.read((char *)&r, sizeof(r));
      v(i, j) = r;
    }
}

template <>
void
dataLoad(std::istream & stream, RealTensorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
    {
      Real r = 0;
      stream.read((char *)&r, sizeof(r));
      v(i, j) = r;
    }
}

template <>
void
dataLoad(std::istream & stream, RealVectorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it
  // won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    Real r = 0;
    stream.read((char *)&r, sizeof(r));
    v(i) = r;
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

  char * s_s = new char[s_size];

  stream.read(s_s, s_size);

  s.write(s_s, s_size);
  delete[] s_s;
}

template <>
void
dataLoad(std::istream & stream, std::stringstream *& s, void * context)
{
  dataLoad(stream, *s, context);
}
