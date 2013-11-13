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

template<>
void
dataStore(std::ostream & stream, Real & v, void * /*context*/)
{
  stream.write((char *) &v, sizeof(v));
}

template<>
void
dataStore(std::ostream & stream, std::string & v, void * /*context*/)
{
  // Write the size of the string
  unsigned int size = v.size();
  stream.write((char *) &size, sizeof(size));

  // Write the string
  stream.write(v.c_str(), sizeof(char)*(size+1));
}


template<>
void
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
void
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
void
dataStore(std::ostream & stream, RealTensorValue & v, void * /*context*/)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; i < LIBMESH_DIM; i++)
      stream.write((char *) &v(i, j), sizeof(v(i, j)));
}

template<>
void
dataStore(std::ostream & stream, RealVectorValue & v, void * /*context*/)
{
  // Obviously if someone loads data with different LIBMESH_DIM than was used for saving them, it won't work.
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    stream.write((char *) &v(i), sizeof(v(i)));
}

template<>
void
dataStore(std::ostream & stream, const Elem * & e, void * context)
{
  // Moose::out<<"const Elem pointer store"<<std::endl;


  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if(e)
  {
    id = e->id();
    // Moose::out<<"Storing Elem id: "<<id<<std::endl;
    if(id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }
  else
  {
    // Moose::out<<"Outputting NULL Elem pointer"<<std::endl;
  }

  storeHelper(stream, id, context);
}

template<>
void
dataStore(std::ostream & stream, const Node * & n, void * context)
{
  // Moose::out<<"const Node pointer store"<<std::endl;


  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if(n)
  {
    id = n->id();
    // Moose::out<<"Storing Node id: "<<id<<std::endl;
    if(id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }
  else
  {
    // Moose::out<<"Outputting NULL Node pointer"<<std::endl;
  }

  storeHelper(stream, id, context);
}


// global load functions

template<>
void
dataLoad(std::istream & stream, Real & v, void * /*context*/)
{
  // Moose::out<<"Real dataLoad"<<std::endl;

  stream.read((char *) &v, sizeof(v));
}

template<>
void
dataLoad(std::istream & stream, std::string & v, void * /*context*/)
{
  // Moose::out<<"std::string dataLoad"<<std::endl;

  // Read the size of the string
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  // Read the string
  char* s = new char[size+1];
  stream.read(s, sizeof(char)*(size+1));

  // Store the string and clean up
  v = s;
  delete[] s;
}

template<>
void
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
void
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
void
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
void
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

template<>
void
dataLoad(std::istream & stream, const Elem * & e, void * context)
{
  if(!context)
    mooseError("Can only load Elem objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // Moose::out<<"Elem pointer load"<<std::endl;

  // TODO: Write out the unique ID of this element
  dof_id_type id;

  loadHelper(stream, id, context);

  if(id != libMesh::DofObject::invalid_id)
  {
    e = mesh->elem(id);
    // Moose::out<<"Retrived Elem: "<<id<<std::endl;
    // Moose::out<<"Elem ptr: "<<e<<std::endl;
    // Moose::out<<"Elem id: "<<e->id()<<std::endl;
  }
  else
  {
    e = NULL;
    // Moose::out<<"NULL Elem"<<std::endl;
  }
}

template<>
void
dataLoad(std::istream & stream, const Node * & n, void * context)
{
  if(!context)
    mooseError("Can only load Node objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // Moose::out<<"Node pointer load"<<std::endl;

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id;

  loadHelper(stream, id, context);

  if(id != libMesh::DofObject::invalid_id)
  {
    n = mesh->nodePtr(id);
    // Moose::out<<"Retrived Node: "<<id<<std::endl;
  }
  else
  {
    n = NULL;
    // Moose::out<<"NULL Node"<<std::endl;
  }
}
