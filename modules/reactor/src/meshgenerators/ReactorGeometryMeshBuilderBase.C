//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorGeometryMeshBuilderBase.h"

InputParameters
ReactorGeometryMeshBuilderBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("A base class that contains common members and methods for Reactor "
                             "Geometry Mesh Builder mesh generators.");

  return params;
}

ReactorGeometryMeshBuilderBase::ReactorGeometryMeshBuilderBase(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
}

void
ReactorGeometryMeshBuilderBase::initializeReactorMeshParams(const std::string reactor_param_name)
{
  bool first_call = _reactor_params != reactor_param_name;
  if (first_call)
    _reactor_params = reactor_param_name;

  // Ensure that the user has supplied a valid ReactorMeshParams object
  if (getMeshByName(_reactor_params) != nullptr)
    mooseError("The reactor_params mesh is not of the correct type");

  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided.");

  // Set parameters related to block name and block / region id mapping
  _current_block_id = getReactorParam<subdomain_id_type>("current_block_id");
  _name_id_map = getReactorParam<std::map<std::string, std::pair<subdomain_id_type, dof_id_type>>>(
      "name_id_map");

  // Set reactor_params_name metadata for use by future mesh generators
  if (first_call)
    declareMeshProperty("reactor_params_name", std::string(_reactor_params));
}

void
ReactorGeometryMeshBuilderBase::initializeReactorMeshParams()
{
  initializeReactorMeshParams(_reactor_params);
}

void
ReactorGeometryMeshBuilderBase::updateReactorMeshParams()
{
  this->declareMeshProperty(_reactor_params, "current_block_id", _current_block_id);
  this->declareMeshProperty(_reactor_params, "name_id_map", _name_id_map);
}

dof_id_type
ReactorGeometryMeshBuilderBase::getBlockId(const std::string block_name,
                                           const dof_id_type region_id)
{
  if (_name_id_map.find(block_name) != _name_id_map.end())
  {
    // Block name exists in map, verify region_id matches one in map and return block_id
    if (_name_id_map[block_name].second != region_id)
      mooseError("Provided region id ",
                 region_id,
                 " for block name ",
                 block_name,
                 " does not match the one that was provided in an earlier mapping - ",
                 _name_id_map[block_name].second);
    return _name_id_map[block_name].first;
  }
  else
  {
    // Block name does not exist, return a new block id and store in _name_id_map
    const auto new_block_id = _current_block_id++;
    _name_id_map[block_name] = std::make_pair(new_block_id, region_id);
    return new_block_id;
  }
}

bool
ReactorGeometryMeshBuilderBase::hasReactorParam(const std::string param_name)
{
  return hasMeshProperty(param_name, _reactor_params);
}
