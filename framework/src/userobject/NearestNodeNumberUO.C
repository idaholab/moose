//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestNodeNumberUO.h"
#include <limits>

registerMooseObject("MooseApp", NearestNodeNumberUO);

InputParameters
NearestNodeNumberUO::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addRequiredParam<Point>("point", "The point");
  params.addClassDescription("Finds and outputs the nearest node number to a point");
  return params;
}

NearestNodeNumberUO::NearestNodeNumberUO(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _my_pid(processor_id()),
    _point(getParam<Point>("point")),
    _node_found(false),
    _min_distance(std::numeric_limits<Real>::max()),
    _closest_node(nullptr),
    _overall_best_id(0)
{
}

void
NearestNodeNumberUO::meshChanged()
{
  _node_found = false;
  _min_distance = std::numeric_limits<Real>::max();
  _min_distance = std::numeric_limits<Real>::max();
  _closest_node = nullptr;
}

void
NearestNodeNumberUO::initialize()
{
  if (!_node_found)
  {
    _min_distance = std::numeric_limits<Real>::max();
    _closest_node = nullptr;
  }
}

void
NearestNodeNumberUO::execute()
{
  if (_node_found)
    return;
  const Real dist = ((*_current_node) - _point).norm();
  if (dist < _min_distance || (dist == _min_distance && _current_node->id() < _closest_node->id()))
  {
    _min_distance = dist;
    _closest_node = _current_node;
  }
}

void
NearestNodeNumberUO::finalize()
{
  Real overall_min_distance = _min_distance;
  gatherMin(overall_min_distance);
  _overall_best_id = (overall_min_distance == _min_distance)
                         ? _closest_node->id()
                         : std::numeric_limits<dof_id_type>::max();
  gatherMin(_overall_best_id);
  _node_found = true;
}

dof_id_type
NearestNodeNumberUO::getClosestNodeId() const
{
  return _overall_best_id;
}

const Node *
NearestNodeNumberUO::getClosestNode() const
{
  if (!_closest_node) // probably no evaluation has occurred
    return nullptr;
  if (_closest_node->id() == _overall_best_id && _closest_node->processor_id() == _my_pid)
    return _closest_node;
  return nullptr;
}

void
NearestNodeNumberUO::threadJoin(const UserObject & y)
{
  const NearestNodeNumberUO & nnn = static_cast<const NearestNodeNumberUO &>(y);
  if (!nnn._closest_node)
    return;
  if (nnn._min_distance < _min_distance ||
      (nnn._min_distance == _min_distance && nnn._closest_node->id() < _closest_node->id()))
  {
    _min_distance = nnn._min_distance;
    _closest_node = nnn._closest_node;
  }
}
