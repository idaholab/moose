//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ModifyNodeGenerator.h"

registerMooseObject("MooseApp", ModifyNodeGenerator);

InputParameters
ModifyNodeGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to modify");

  params.addRequiredParam<std::vector<int>>("node_id", "Id of modified node");

  params.addRequiredParam<std::vector<Point>>("new_position", "New position in vector space");

  params.addClassDescription("Modifies the position of one node");

  return params;
}

ModifyNodeGenerator::ModifyNodeGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _node_id(getParam<std::vector<int>>("node_id")),
    _new_position(getParam<std::vector<Point>>("new_position"))
{
}

std::unique_ptr<MeshBase>
ModifyNodeGenerator::generate()
{
  if (_node_id.size() != _new_position.size())
    mooseError("Node ids and new positions sizes do not match. Please check your input");
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // get the node
  for (int i = 0; i < int(_node_id.size()); i++)
  {
    auto * node = mesh->query_node_ptr(_node_id[i]);

    // change the position of the acquired node
    node->assign(_new_position[i]);
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
