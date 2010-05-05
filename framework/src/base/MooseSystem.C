#include "MooseSystem.h"
#include "ElementData.h"
#include "FaceData.h"
#include "AuxData.h"
#include "AuxKernel.h"
#include "AuxFactory.h"


//libMesh includes
#include "numeric_vector.h"
#include "mesh.h"
#include "exodusII_io.h"

MooseSystem::MooseSystem()
  :_es(NULL),
   _system(NULL),
   _aux_system(NULL),
   _mesh(NULL),
   _dim(0),
   _exreader(NULL),
   _is_valid(false)
{}

MooseSystem::~MooseSystem()
{
  if (_is_valid)
    for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid) 
    {
      delete _element_data[tid];
      delete _face_data[tid];
      delete _aux_data[tid];
    }

  if (_es != NULL)
    delete _es;

  if (_mesh != NULL)
    delete _mesh;
}

MeshBase *
MooseSystem::initMesh(unsigned int dim) 
{
  if (_mesh != NULL)
    mooseError("Mesh already initialized for this MooseSystem");

  _dim = dim;
  _mesh = new Mesh(dim);
  return _mesh;
}

MeshBase *
MooseSystem::getMesh() 
{
  checkValid();
  return _mesh;
}

EquationSystems *
MooseSystem::getEquationSystems()
{
//  checkValid();
  return _es;
}

TransientNonlinearImplicitSystem *
MooseSystem::getNonlinearSystem()
{
  checkValid();
  return _system;
}

TransientExplicitSystem *
MooseSystem::getAuxSystem()
{
  checkValid();
  return _aux_system;
}

EquationSystems *
MooseSystem::initEquationSystems()
{
  if (_es != NULL)
    mooseError("EquationSystems Object already initialized for this MooseSystem");
  
  _es = new EquationSystems(*_mesh);
  _system = &_es->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _aux_system = &_es->add_system<TransientExplicitSystem>("AuxiliarySystem");
  
  return _es;
}

void
MooseSystem::initDataStructures()
{
  if (_mesh == NULL)
    mooseError("Mesh is unintialized in call to initialize data structures");
  if (_es == NULL)
    mooseError("EquationsSystems is unintialized in call to initialize data structures");

  unsigned int n_threads = libMesh::n_threads();
  _element_data.resize(n_threads);
  _face_data.resize(n_threads);
  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _element_data[tid] = new ElementData(*this);
    _face_data[tid] = new FaceData(*this);
    _aux_data[tid] = new AuxData(*this, *_element_data[tid]);
  }

  // Need to initialize data
  _is_valid = true;
}

void
MooseSystem::checkValid()
{
  if (!_is_valid)
    mooseError("MooseSystem has not been properly initialized before accessing data");
}

ElementData *
MooseSystem::getElementData(THREAD_ID tid)
{
  checkValid();
  return _element_data[tid];
}

FaceData *
MooseSystem::getFaceData(THREAD_ID tid)
{
  checkValid();
  return _face_data[tid];
}

AuxData *
MooseSystem::getAuxData(THREAD_ID tid)
{
  checkValid();
  return _aux_data[tid];
}

ExodusII_IO *
MooseSystem::getExodusReader()
{
  if(!_exreader)
    _exreader = new ExodusII_IO(*Moose::mesh);

  return _exreader;
}

