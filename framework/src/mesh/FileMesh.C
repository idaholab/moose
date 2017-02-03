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
#include "MooseApp.h"

// libMesh includes
#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/checkpoint_io.h"
#include "libmesh/parallel_mesh.h"

template<>
InputParameters validParams<FileMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read");
  params.addClassDescription("Read a mesh from a file.");

  return params;
}

FileMesh::FileMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _exreader(nullptr)
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

FileMesh::FileMesh(const FileMesh & other_mesh) :
    MooseMesh(other_mesh),
    _file_name(other_mesh._file_name),
    _exreader(nullptr)
{
}

FileMesh::~FileMesh()
{
}

MooseMesh &
FileMesh::clone() const
{
  return *(new FileMesh(*this));
}

void
FileMesh::buildMesh()
{
  Moose::perf_log.push("Read Mesh", "Setup");

   std::string _file_name = getParam<MeshFileName>("file");

   // If we have an exodus file and it's pre-split...
   bool is_nemesis = _pre_split && (_file_name.rfind(".exd") < _file_name.size() || _file_name.rfind(".e") < _file_name.size());

   // If we have a checkpoint file and it's pre-split...
   bool is_checkpoint = _pre_split && _file_name.rfind(".cpr") < _file_name.size();

  if (is_nemesis)
  {
    // Nemesis_IO only takes a reference to DistributedMesh, so we can't be quite so short here.
    DistributedMesh & pmesh = cast_ref<DistributedMesh &>(getMesh());
    Nemesis_IO(pmesh).read(_file_name);

    getMesh().allow_renumbering(false);

    // Even if we want repartitioning when load balancing later, we'll
    // begin with the default partitioning defined by the Nemesis
    // file.
    bool skip_partitioning_later = getMesh().skip_partitioning();
    getMesh().skip_partitioning(true);
    getMesh().prepare_for_use();
    getMesh().skip_partitioning(skip_partitioning_later);
  }
  else if (is_checkpoint)
  {
    CheckpointIO reader(getMesh());

    reader.binary() = true;

    auto & mesh = getMesh();

    DistributedMesh * pmesh = cast_ptr<DistributedMesh *>(&mesh);

    if (pmesh)
      reader.parallel() = true;

    reader.read(_file_name);

    mesh.skip_partitioning(true);
    mesh.prepare_for_use();
  }
  else // not reading Nemesis files
  {
    MooseUtils::checkFileReadable(_file_name);

    // See if the user has requested reading a solution from the file.  If so, we'll need to read
    // the mesh with the exodus reader instead of using mesh.read().  This will read the mesh on
    // every processor

    if (_app.setFileRestart() && (_file_name.rfind(".exd") < _file_name.size() || _file_name.rfind(".e") < _file_name.size()))
    {
      _exreader = libmesh_make_unique<ExodusII_IO>(getMesh());
      _exreader->read(_file_name);

      getMesh().allow_renumbering(false);
      getMesh().prepare_for_use();
    }
    else
      getMesh().read(_file_name);
  }

  Moose::perf_log.pop("Read Mesh", "Setup");
}
