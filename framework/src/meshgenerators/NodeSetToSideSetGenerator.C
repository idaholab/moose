//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeSetToSideSetGenerator.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", NodeSetToSideSetGenerator);

InputParameters
NodeSetToSideSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Mesh generator which constructs side sets from node sets, and vise versa");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Input mesh the operation will be applied to");
  return params;
}

NodeSetToSideSetGenerator::NodeSetToSideSetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
NodeSetToSideSetGenerator::generate()
{
  _input->get_boundary_info().build_side_list_from_node_list();

  return dynamic_pointer_cast<MeshBase>(_input);
}
