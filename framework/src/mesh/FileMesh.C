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

#include "FileMesh.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"

// libMesh includes
#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"

template<>
InputParameters validParams<FileMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read");
  params.addParam<bool>("nemesis", false, "If nemesis=true and file=foo.e, actually reads foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, where N = # CPUs, with NemesisIO.");
  params.addParam<bool>("skip_partitioning", false, "If true the mesh won't be partitioned.  Probably not a good idea to use it with a serial mesh!");

  // groups
  params.addParamNamesToGroup("nemesis", "Advanced");
  params.addParamNamesToGroup("skip_partitioning", "Partitioning");

  return params;
}


FileMesh::FileMesh(const std::string & name, InputParameters parameters) :
    MooseMesh(name, parameters),
    _file_name(getParam<MeshFileName>("file")),
    _exreader(NULL)
{
  _mesh.set_mesh_dimension(getParam<MooseEnum>("dim"));
  _is_parallel = getParam<bool>("nemesis");
}

FileMesh::FileMesh(const FileMesh & other_mesh) :
    MooseMesh(other_mesh),
    _file_name(other_mesh._file_name),
    _exreader(NULL)
{
}

FileMesh::~FileMesh()
{
  delete _exreader;
}

MooseMesh &
FileMesh::clone() const
{
  return *(new FileMesh(*this));
}

void
FileMesh::init()
{
//  mooseAssert(_mesh == NULL, "Mesh already exists, and you are trying to read another");
  std::string _file_name = getParam<MeshFileName>("file");

  Moose::setup_perf_log.push("Read Mesh","Setup");
  if (getParam<bool>("nemesis"))
  {
    // Nemesis_IO only takes a reference to ParallelMesh, so we can't be quite so short here.
    ParallelMesh& pmesh = libmesh_cast_ref<ParallelMesh&>(getMesh());
    Nemesis_IO(pmesh).read(_file_name);
  }
  else // not reading Nemesis files
  {
    MooseUtils::checkFileReadable(_file_name);

    // if reading ExodusII, read it through a reader and save it off, since it will be used in possible "copy nodal vars" action
    // NOTE: the other reader that can do copy nodal values is GMVIO, but GMV is _pretty_ old right now (May 2011)
    // NOTE: Actually we have two dependencies on the raw exodus reader now - we also need it for named sideset support
    // when distributing the mesh in parallel (Feb 2013)
    if (_file_name.rfind(".exd") < _file_name.size() ||
        _file_name.rfind(".e") < _file_name.size())
    {
      _exreader = new ExodusII_IO(_mesh);
      _exreader->read(_file_name);
    }
    else
      _mesh.read(_file_name);
  }

  _mesh.skip_partitioning(getParam<bool>("skip_partitioning"));

  Moose::setup_perf_log.pop("Read Mesh","Setup");
}

void
FileMesh::read(const std::string & file_name)
{
  if (dynamic_cast<ParallelMesh *>(&_mesh) && !_is_parallel)
    _mesh.read(file_name, NULL, false);
  else
    _mesh.read(file_name, NULL, true);
}
