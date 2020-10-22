//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckMeshMetaDataAction.h"

registerMooseAction("MooseTestApp", CheckMeshMetaDataAction, "determine_system_type");

InputParameters
CheckMeshMetaDataAction::validParams()
{
  auto params = Action::validParams();
  params.addRequiredParam<std::string>("mesh_generator_name",
                                       "The mesh generator providing the mesh meta data");
  params.addRequiredParam<std::string>("mesh_meta_data_name", "The name of the mesh meta data");
  return params;
}

CheckMeshMetaDataAction::CheckMeshMetaDataAction(const InputParameters & params) : Action(params) {}

void
CheckMeshMetaDataAction::act()
{
  auto & mesher_name = getParam<std::string>("mesh_generator_name");
  auto & data_name = getParam<std::string>("mesh_meta_data_name");
  if (!hasMeshProperty(data_name, mesher_name))
    mooseError(mesher_name, " generator does not provide the mesh meta data with name ", data_name);
  else
    _console << "Mesh meta data " << data_name << " is provided by mesh generator " << mesher_name
             << "." << std::endl;
}
