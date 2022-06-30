//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoveNodeGenerator.h"
#include "CastUniquePointer.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", MoveNodeGenerator);

InputParameters
MoveNodeGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to Move");

  params.addRequiredParam<std::vector<dof_id_type>>("node_id", "Id of modified node");

  params.addParam<std::vector<Point>>("new_position", "New position in vector space");
  params.addParam<std::vector<Point>>("shift_position",
                                      "Shifts to apply to the position in vector space");

  params.addClassDescription("Modifies the position of one or more nodes");

  return params;
}

MoveNodeGenerator::MoveNodeGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _node_id(getParam<std::vector<dof_id_type>>("node_id")),
    _new_position(nullptr),
    _shift_position(nullptr)
{
  if (isParamValid("shift_position") == isParamValid("new_position"))
    mooseError("You must specify either 'shift_position' or 'new_position'! "
               "You have specified either both or none");

  if (isParamValid("new_position"))
  {
    _new_position = &getParam<std::vector<Point>>("new_position");

    if (_node_id.size() != _new_position->size())
      paramError("node_id", "Must be the same length as 'new_position'.");
  }

  if (isParamValid("shift_position"))
  {
    _shift_position = &getParam<std::vector<Point>>("shift_position");

    if (_node_id.size() != _shift_position->size())
      paramError("node_id", "Must be the same length as 'shift_position'.");
  }
}

std::unique_ptr<MeshBase>
MoveNodeGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // get the node
  for (MooseIndex(_node_id.size()) i = 0; i < _node_id.size(); i++)
  {
    std::size_t num_found = 0;
    auto * node = mesh->query_node_ptr(_node_id[i]);

    // change the position of the acquired node
    if (node)
    {
      if (_new_position)
        node->assign((*_new_position)[i]);
      else
      {
        Point pt((*node)(0), (*node)(1), (*node)(2));
        node->assign(pt + (*_shift_position)[i]);
      }

      ++num_found;
    }

    // Make sure we found the node
    mesh->comm().sum(num_found);
    if (!num_found)
      mooseError("A node with the ID ", _node_id[i], " was not found.");
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
