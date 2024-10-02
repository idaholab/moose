//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradientJumpIndicator.h"

registerMooseObject("MooseApp", GradientJumpIndicator);

InputParameters
GradientJumpIndicator::validParams()
{
  InputParameters params = InternalSideIndicator::validParams();
  params.addClassDescription(
      "Compute the jump of the solution gradient across element boundaries.");
  params.addParam<bool>("variable_is_FV",
                        false,
                        "Whether the solution variable is using a finite volume discretization");

  // We need more ghosting to compute finite volume gradients across from a boundary
  // We do not use skewness correction here, therefore avoiding needing three layers
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::ALGEBRAIC,
      [](const InputParameters & obj_params, InputParameters & rm_params) {
        rm_params.set<unsigned short>("layers") = obj_params.get<bool>("variable_is_FV") ? 2 : 1;
      });

  return params;
}

GradientJumpIndicator::GradientJumpIndicator(const InputParameters & parameters)
  : InternalSideIndicator(parameters)
{
}

Real
GradientJumpIndicator::computeQpIntegral()
{
  Real jump = 0;
  // If the variable is not defined in the neighbor cell, we cant define the block
  // If the indicator is not defined in the neighbor cell, we should not be looking at it
  mooseAssert(_neighbor_elem, "Should have a neighbor");
  if (_var.hasBlocks(_neighbor_elem->subdomain_id()) && hasBlocks(_neighbor_elem->subdomain_id()))
  {
    if (_var.isFV())
      jump =
          (MetaPhysicL::raw_value(_var.gradient(
               Moose::ElemArg{_current_elem, /*correct_skewness=*/false}, Moose::currentState())) -
           MetaPhysicL::raw_value(
               _var.gradient(Moose::ElemArg{_neighbor_elem, false}, Moose::currentState()))) *
          _normals[_qp];
    else
      jump = (_grad_u[_qp] - _grad_u_neighbor[_qp]) * _normals[_qp];
  }

  return jump * jump;
}
