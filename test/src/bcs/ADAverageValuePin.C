//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAverageValuePin.h"
#include "MooseMesh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"

registerMooseObject("MooseTestApp", ADAverageValuePin);

InputParameters
ADAverageValuePin::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addRelationshipManager("GhostEverything",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);
  return params;
}

ADAverageValuePin::ADAverageValuePin(const InputParameters & parameters) : ADNodalBC(parameters) {}

ADReal
ADAverageValuePin::computeQpResidual()
{
  ADReal sum = 0;
  for (const auto * const node :
       as_range(_mesh.getMesh().active_nodes_begin(), _mesh.getMesh().active_nodes_end()))
  {
    const auto dof_index = node->dof_number(_sys.number(), _var.number(), 0);
    ADReal soln = (*_sys.currentSolution())(dof_index);
    Moose::derivInsert(soln.derivatives(), dof_index, 1.);
    sum += soln;
  }
  return sum;
}
