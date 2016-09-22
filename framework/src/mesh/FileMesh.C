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
  params.addParam<bool>("checkpoint", false, "Whether or not the file format is a checkpoint file");
  return params;
}

FileMesh::FileMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _is_checkpoint(getParam<bool>("checkpoint")),
    _exreader(NULL)
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

FileMesh::FileMesh(const FileMesh & other_mesh) :
    MooseMesh(other_mesh),
    _file_name(other_mesh._file_name),
    _is_checkpoint(other_mesh._is_checkpoint),
    _exreader(NULL)
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
  std::string _file_name = getParam<MeshFileName>("file");

  Moose::perf_log.push("Read Mesh", "Setup");
  if (_is_nemesis)
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
  else if (_is_checkpoint)
  {
    CheckpointIO reader(getMesh());

    reader.binary() = true;

    auto & mesh = getMesh();

    DistributedMesh * pmesh = cast_ptr<DistributedMesh *>(&mesh);

    //mesh.allow_renumbering(false);

    if (pmesh)
      reader.parallel() = true;

    reader.read(_file_name);

    //mesh.update_parallel_id_counts();
    //mesh.find_neighbors(true, true);
    //mesh.update_parallel_id_counts();
    //mesh.delete_remote_elements();
    mesh.skip_partitioning(true);
    //mesh.redistribute();
    mesh.prepare_for_use();
    mesh.print_info();
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

void
FileMesh::read(const std::string & file_name)
{
  if (dynamic_cast<DistributedMesh *>(&getMesh()) && !_is_nemesis)
    getMesh().read(file_name, /*mesh_data=*/NULL, /*skip_renumber=*/false);
  else
    getMesh().read(file_name, /*mesh_data=*/NULL, /*skip_renumber=*/true);
}
