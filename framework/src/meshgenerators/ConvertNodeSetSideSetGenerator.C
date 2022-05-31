//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvertNodeSetSideSetGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_refinement.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", ConvertNodeSetSideSetGenerator);

InputParameters
ConvertNodeSetSideSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which refines one or more blocks in an existing mesh");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to refine");
  params.addRequiredParam<bool>(
      "convert_node_list_from_side_list",
      "Toggles whether side entities should be transcribed into nodes.");
  params.addRequiredParam<bool>(
      "convert_side_list_from_node_list",
      "Toggles whether node entities should be transcribed into sides.");

  return params;
}

ConvertNodeSetSideSetGenerator::ConvertNodeSetSideSetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _convert_side_list_from_node_list(getParam<bool>("convert_side_list_from_node_list")),
    _convert_node_list_from_side_list(getParam<bool>("convert_node_list_from_side_list"))
{

}

std::unique_ptr<MeshBase>
ConvertNodeSetSideSetGenerator::generate()
{
  if(_convert_node_list_from_side_list)
    _input->get_boundary_info().build_node_list_from_side_list();

  if(_convert_side_list_from_node_list)
    _input->get_boundary_info().build_side_list_from_node_list();

  return dynamic_pointer_cast<MeshBase>(_input);
}
