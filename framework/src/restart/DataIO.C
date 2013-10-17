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
  //std::cout<<"Real dataStore"<<std::endl;

  stream.write((char *) &v, sizeof(v));
}

template<>
void
dataStore(std::ostream & stream, std::string & v, void * /*context*/)
{
  //std::cout<<"std::string dataStore ("<<v<<")"<<std::endl;
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
  // std::cout<<"const Elem pointer store"<<std::endl;


  // TODO: Write out the unique ID of this elem
  dof_id_type id = libMesh::DofObject::invalid_id;

  if(e)
  {
    id = e->id();
    // std::cout<<"Storing Elem id: "<<id<<std::endl;
    if(id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Elems with invalid ids!");
  }
  else
  {
    // std::cout<<"Outputting NULL Elem pointer"<<std::endl;
  }

  storeHelper(stream, id, context);
}

template<>
void
dataStore(std::ostream & stream, const Node * & n, void * context)
{
  // std::cout<<"const Node pointer store"<<std::endl;


  // TODO: Write out the unique ID of this node
  dof_id_type id = libMesh::DofObject::invalid_id;

  if(n)
  {
    id = n->id();
    // std::cout<<"Storing Node id: "<<id<<std::endl;
    if(id == libMesh::DofObject::invalid_id)
      mooseError("Can't output Nodes with invalid ids!");
  }
  else
  {
    // std::cout<<"Outputting NULL Node pointer"<<std::endl;
  }

  storeHelper(stream, id, context);
}


// global load functions

template<>
void
dataLoad(std::istream & stream, Real & v, void * /*context*/)
{
  // std::cout<<"Real dataLoad"<<std::endl;

  stream.read((char *) &v, sizeof(v));
}

template<>
void
dataLoad(std::istream & stream, std::string & v, void * /*context*/)
{
  // std::cout<<"std::string dataLoad"<<std::endl;

  // Read the size of the string
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // std::cout<<"Size: "<<size<<std::endl;

  // Read and store the string
  char* s = new char[size];
  stream.read(s, sizeof(char)*(size+1));
  v = s;

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

  // std::cout<<"Elem pointer load"<<std::endl;

  // TODO: Write out the unique ID of this element
  dof_id_type id;

  loadHelper(stream, id, context);

  if(id != libMesh::DofObject::invalid_id)
  {
    e = mesh->elem(id);
    // std::cout<<"Retrived Elem: "<<id<<std::endl;
    // std::cout<<"Elem ptr: "<<e<<std::endl;
    // std::cout<<"Elem id: "<<e->id()<<std::endl;
  }
  else
  {
    e = NULL;
    // std::cout<<"NULL Elem"<<std::endl;
  }
}

template<>
void
dataLoad(std::istream & stream, const Node * & n, void * context)
{
  if(!context)
    mooseError("Can only load Node objects using a MooseMesh context!");

  MooseMesh * mesh = static_cast<MooseMesh *>(context);

  // std::cout<<"Node pointer load"<<std::endl;

  // TODO: Write out the unique ID of this nodeent
  dof_id_type id;

  loadHelper(stream, id, context);

  if(id != libMesh::DofObject::invalid_id)
  {
    n = mesh->nodePtr(id);
    // std::cout<<"Retrived Node: "<<id<<std::endl;
  }
  else
  {
    n = NULL;
    // std::cout<<"NULL Node"<<std::endl;
  }
}
