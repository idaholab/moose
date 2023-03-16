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
  _reactor_params = reactor_param_name;

  // Ensure that the user has supplied a valid ReactorMeshParams object
  _reactor_params_mesh = &getMeshByName(reactor_param_name);
  if (*_reactor_params_mesh)
    mooseError("The reactor_params mesh is not of the correct type");

  if (!hasMeshProperty<int>("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty<std::string>("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided.");

  // Set reactor_params_name metadata for use by future mesh generators
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));
}

void
ReactorGeometryMeshBuilderBase::freeReactorMeshParams()
{
  _reactor_params_mesh->reset();
}

unsigned int
ReactorGeometryMeshBuilderBase::getElemIntegerFromMesh(MeshBase & input_mesh,
                                                       std::string extra_int_name,
                                                       bool should_exist)
{
  if (input_mesh.has_elem_integer(extra_int_name))
    return input_mesh.get_elem_integer_index(extra_int_name);
  else
  {
    if (should_exist)
      mooseError("Expected extruded mesh to have " + extra_int_name + " extra integers");
    else
      return input_mesh.add_elem_integer(extra_int_name);
  }
}

void
ReactorGeometryMeshBuilderBase::updateElementBlockNameId(
    MeshBase & input_mesh,
    Elem * elem,
    std::map<std::string, SubdomainID> & name_id_map,
    std::string elem_block_name,
    SubdomainID & next_free_id)
{
  SubdomainID elem_block_id;
  if (name_id_map.find(elem_block_name) == name_id_map.end())
  {
    // Block name does not exist in mesh yet, assign new block id and name
    elem_block_id = next_free_id++;
    elem->subdomain_id() = elem_block_id;
    input_mesh.subdomain_name(elem_block_id) = elem_block_name;
    name_id_map[elem_block_name] = elem_block_id;
  }
  else
  {
    // Block name exists in mesh, reuse block id
    elem_block_id = name_id_map[elem_block_name];
    elem->subdomain_id() = elem_block_id;
  }
}
