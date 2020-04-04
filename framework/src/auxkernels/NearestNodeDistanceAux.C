//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestNodeDistanceAux.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", NearestNodeDistanceAux);

InputParameters
NearestNodeDistanceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Stores the distance between a block and boundary or between two boundaries.");
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to find the distance to.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

NearestNodeDistanceAux::NearestNodeDistanceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nearest_node(_nodal ? getNearestNodeLocator(parameters.get<BoundaryName>("paired_boundary"),
                                                 boundaryNames()[0])
                         : getQuadratureNearestNodeLocator(
                               parameters.get<BoundaryName>("paired_boundary"), boundaryNames()[0]))
{
  if (boundaryNames().size() > 1)
    mooseError("NearestNodeDistanceAux can only be used with one boundary at a time!");
}

Real
NearestNodeDistanceAux::computeValue()
{
  if (_nodal)
    return _nearest_node.distance(_current_node->id());

  Node * qnode = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  return _nearest_node.distance(qnode->id());
}
