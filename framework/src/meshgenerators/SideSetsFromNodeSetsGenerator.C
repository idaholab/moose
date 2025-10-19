//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromNodeSetsGenerator.h"

#include "CastUniquePointer.h"
#include "libmesh/boundary_info.h"
#include "libmesh/elem_side_builder.h"

registerMooseObject("MooseApp", SideSetsFromNodeSetsGenerator);

InputParameters
SideSetsFromNodeSetsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which constructs side sets from node sets");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Input mesh the operation will be applied to");
  params.addParam<std::vector<BoundaryName>>(
      "nodesets_to_convert",
      "If specified, list of nodesets to convert. If not specified, all nodesets are converted");
  return params;
}

SideSetsFromNodeSetsGenerator::SideSetsFromNodeSetsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SideSetsFromNodeSetsGenerator::generate()
{
  if (!isParamValid("nodesets_to_convert"))
    _input->get_boundary_info().build_side_list_from_node_list();
  else
    _input->get_boundary_info().build_side_list_from_node_list(
        getParam<std::vector<BoundaryName>>("nodesets_to_convert"));

  return dynamic_pointer_cast<MeshBase>(_input);
}
