//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMesh.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"

template <>
InputParameters
validParams<FileMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read");
  params.addClassDescription("Read a mesh from a file.");
  return params;
}

FileMesh::FileMesh(const InputParameters & parameters)
  : MooseMesh(parameters), _file_name(getParam<MeshFileName>("file"))
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

FileMesh::FileMesh(const FileMesh & other_mesh)
  : MooseMesh(other_mesh), _file_name(other_mesh._file_name)
{
}

FileMesh::~FileMesh() {}

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
  else // not reading Nemesis files
  {
    // See if the user has requested reading a solution from the file.  If so, we'll need to read
    // the mesh with the exodus reader instead of using mesh.read().  This will read the mesh on
    // every processor

    if (_app.setFileRestart() && (_file_name.rfind(".exd") < _file_name.size() ||
                                  _file_name.rfind(".e") < _file_name.size()))
    {
      MooseUtils::checkFileReadable(_file_name);

      _exreader = libmesh_make_unique<ExodusII_IO>(getMesh());
      _exreader->read(_file_name);

      getMesh().allow_renumbering(false);
      getMesh().prepare_for_use();
    }
    else
    {
      auto slash_pos = _file_name.find_last_of("/");
      auto path = _file_name.substr(0, slash_pos);
      auto file = _file_name.substr(slash_pos + 1);

      bool restarting;
      // If we are reading a mesh while restarting, then we might have
      // a solution file that relies on that mesh partitioning and/or
      // numbering.  In that case, we need to turn off repartitioning
      // and renumbering, at least at first.
      if (file == "LATEST")
      {
        std::list<std::string> dir_list(1, path);
        std::list<std::string> files = MooseUtils::getFilesInDirs(dir_list);

        // Fill in the name of the LATEST file so we can open it and read it.
        _file_name = MooseUtils::getLatestMeshCheckpointFile(files);
        restarting = true;
      }
      else
        restarting = _file_name.rfind(".cpa") < _file_name.size() ||
                     _file_name.rfind(".cpr") < _file_name.size();

      const bool skip_partitioning_later = restarting && getMesh().skip_partitioning();
      const bool allow_renumbering_later = restarting && getMesh().allow_renumbering();

      if (restarting)
      {
        getMesh().skip_partitioning(true);
        getMesh().allow_renumbering(false);
      }

      MooseUtils::checkFileReadable(_file_name);
      getMesh().read(_file_name);

      if (restarting)
      {
        getMesh().allow_renumbering(allow_renumbering_later);
        getMesh().skip_partitioning(skip_partitioning_later);
      }
    }
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
