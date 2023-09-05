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
  MooseEnum types("map_BoundaryID_RealVectorValue");
  params.addRequiredParam<MooseEnum>(
      "mesh_meta_data_type", types, "The type of the mesh meta data");
  return params;
}

CheckMeshMetaDataAction::CheckMeshMetaDataAction(const InputParameters & params) : Action(params) {}

void
CheckMeshMetaDataAction::act()
{
  const auto & mesher_name = getParam<std::string>("mesh_generator_name");
  const auto & data_name = getParam<std::string>("mesh_meta_data_name");
  const auto & data_type = getParam<MooseEnum>("mesh_meta_data_type");

  bool has_value = false;
  if (data_type == "map_BoundaryID_RealVectorValue")
    has_value = hasMeshProperty<std::map<BoundaryID, RealVectorValue>>(data_name, mesher_name);

  if (!has_value)
    mooseError(mesher_name, " does not provide meta data ", data_name, " of enum type ", data_type);

  _console << "Mesh meta data " << data_name << " of enum type " << data_type << " is provided by "
           << mesher_name << std::endl;
}
