//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshMetaDataReporter.h"

registerMooseObject("MooseApp", MeshMetaDataReporter);

InputParameters
MeshMetaDataReporter::validParams()
{
  auto params = RestartableDataReporter::validParams();

  params.addClassDescription("Reports the mesh meta data.");

  params.set<std::string>("map") = MooseApp::MESH_META_DATA;
  params.suppressParameter<std::string>("map");

  params.set<ExecFlagEnum>("execute_on") = "initial";

  return params;
}

MeshMetaDataReporter::MeshMetaDataReporter(const InputParameters & parameters)
  : RestartableDataReporter(parameters)
{
}
