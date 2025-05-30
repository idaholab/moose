//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeSetsFromSideSetsGenerator.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", NodeSetsFromSideSetsGenerator);

InputParameters
NodeSetsFromSideSetsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which constructs node sets from side sets");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Input mesh the operation will be applied to");

  return params;
}

NodeSetsFromSideSetsGenerator::NodeSetsFromSideSetsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
NodeSetsFromSideSetsGenerator::generate()
{
  _input->get_boundary_info().build_node_list_from_side_list();

  return dynamic_pointer_cast<MeshBase>(_input);
}
