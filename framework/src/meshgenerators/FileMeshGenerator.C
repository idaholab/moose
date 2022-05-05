//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshGenerator.h"
#include "CastUniquePointer.h"
#include "RestartableDataIO.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", FileMeshGenerator);

InputParameters
FileMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshFileName>("file", "The filename to read.");
  params.addParam<std::vector<std::string>>(
      "exodus_extra_element_integers",
      "The variable names in the mesh file for loading extra element integers");
  params.addParam<bool>("use_for_exodus_restart",
                        false,
                        "True to indicate that the mesh file this generator is reading can be used "
                        "for restarting variables");
  params.addParam<bool>("skip_partitioning",
                        false,
                        "True to skip partitioning, only after this mesh generator, "
                        "because the mesh was pre-split for example.");
  params.addParam<bool>("allow_renumbering",
                        true,
                        "Whether to allow the mesh to renumber nodes and elements. Note that this "
                        "parameter is only relevant for non-exodus files, e.g. if reading from "
                        "checkpoint for example. For exodus we always disallow renumbering.");
  params.addParam<bool>("clear_spline_nodes",
                        false,
                        "If clear_spline_nodes=true, IsoGeometric Analyis spline nodes "
                        "and constraints are removed from an IGA mesh, after which only "
                        "C^0 Rational-Bernstein-Bezier elements will remain.");
  params.addClassDescription("Read a mesh from a file.");
  return params;
}

FileMeshGenerator::FileMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _skip_partitioning(getParam<bool>("skip_partitioning")),
    _allow_renumbering(getParam<bool>("allow_renumbering"))
{
}

std::unique_ptr<MeshBase>
FileMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  // Figure out if we are reading an Exodus file, but not Tetgen (*.ele)
  bool exodus = (_file_name.rfind(".exd") < _file_name.size() ||
                 _file_name.rfind(".e") < _file_name.size()) &&
                _file_name.rfind(".ele") == std::string::npos;
  bool has_exodus_integers = isParamValid("exodus_extra_element_integers");
  bool restart_exodus = (getParam<bool>("use_for_exodus_restart") && _app.getExodusFileRestart());
  if (exodus)
  {
    auto exreader = std::make_shared<ExodusII_IO>(*mesh);

    if (has_exodus_integers)
      exreader->set_extra_integer_vars(
          getParam<std::vector<std::string>>("exodus_extra_element_integers"));

    if (restart_exodus)
    {
      _app.setExReaderForRestart(std::move(exreader));
      exreader->read(_file_name);
      mesh->allow_renumbering(false);
    }
    else
    {
      if (mesh->processor_id() == 0)
      {
        exreader->read(_file_name);

        if (getParam<bool>("clear_spline_nodes"))
          MeshTools::clear_spline_nodes(*mesh);
      }
      MeshCommunication().broadcast(*mesh);
    }
    // Skip partitioning if the user requested it
    if (_skip_partitioning)
      mesh->skip_partitioning(true);
    mesh->prepare_for_use();
    mesh->skip_partitioning(false);
  }
  else
  {
    if (_pars.isParamSetByUser("exodus_extra_element_integers"))
      mooseError("\"exodus_extra_element_integers\" should be given only for Exodus mesh files");
    if (_pars.isParamSetByUser("use_for_exodus_restart"))
      mooseError("\"use_for_exodus_restart\" should be given only for Exodus mesh files");

    // to support LATEST word for loading checkpoint files
    std::string file_name = MooseUtils::convertLatestCheckpoint(_file_name, false);

    mesh->skip_partitioning(_skip_partitioning);
    mesh->allow_renumbering(_allow_renumbering);
    mesh->read(file_name);

    // we also read declared mesh meta data here if there is meta data file
    RestartableDataIO restartable(_app);
    std::string fname = file_name + "/meta_data_mesh" + restartable.getRestartableDataExt();
    if (MooseUtils::pathExists(fname))
    {
      restartable.setErrorOnLoadWithDifferentNumberOfProcessors(false);
      // get reference to mesh meta data (created by MooseApp)
      auto & meta_data = _app.getRestartableDataMap(MooseApp::MESH_META_DATA);
      if (restartable.readRestartableDataHeaderFromFile(fname, false))
        restartable.readRestartableData(meta_data, DataNames());
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
