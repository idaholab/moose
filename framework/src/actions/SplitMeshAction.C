//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitMeshAction.h"

#include "MooseApp.h"
#include "MooseUtils.h"
#include "MooseMesh.h"
#include "libmesh/checkpoint_io.h"

registerMooseAction("MooseApp", SplitMeshAction, "split_mesh");

template <>
InputParameters
validParams<SplitMeshAction>()
{
  return validParams<Action>();
}

SplitMeshAction::SplitMeshAction(InputParameters params) : Action(params) {}

void
SplitMeshAction::act()
{
  auto mesh = _app.actionWarehouse().mesh();

  if (mesh->getFileName() == "" && _app.parameters().get<std::string>("split_file") == "")
    mooseError("Output mesh file name must be specified (with --split-file) when splitting "
               "non-file-based meshes");

  auto splitstr = _app.parameters().get<std::string>("split_mesh");
  std::vector<unsigned int> splits;
  bool success = MooseUtils::tokenizeAndConvert(splitstr, splits, ", ");
  if (!success)
    mooseError("invalid argument for --split-mesh: '", splitstr, "'");

  for (std::size_t i = 0; i < splits.size(); i++)
  {
    processor_id_type n = splits[i];
    Moose::out << "Splitting " << n << " ways..." << std::endl;

    auto cpr = libMesh::split_mesh(*mesh, n);
    Moose::out << "    - writing " << cpr->current_processor_ids().size() << " files per process..."
               << std::endl;
    cpr->binary() = true;
    auto fname = mesh->getFileName();
    if (fname == "")
      fname = _app.parameters().get<std::string>("split_file");
    fname = MooseUtils::stripExtension(fname) + ".cpr";
    cpr->write(fname);
  }
}
