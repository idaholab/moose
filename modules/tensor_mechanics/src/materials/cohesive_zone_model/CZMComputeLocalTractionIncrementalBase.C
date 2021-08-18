//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeLocalTractionIncrementalBase.h"

InputParameters
CZMComputeLocalTractionIncrementalBase::validParams()
{
  InputParameters params = CZMComputeLocalTractionBase::validParams();

  params.addClassDescription(
      "Base class for implementing incremental cohesive zone constituive material models");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMComputeLocalTractionIncrementalBase::CZMComputeLocalTractionIncrementalBase(
    const InputParameters & parameters)
  : CZMComputeLocalTractionBase(parameters),
    _interface_traction_inc(
        declarePropertyByName<RealVectorValue>(_base_name + "interface_traction_inc")),
    _interface_traction_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_traction")),
    _interface_displacement_jump_inc(
        declarePropertyByName<RealVectorValue>(_base_name + "interface_displacement_jump_inc")),
    _interface_displacement_jump_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_displacement_jump"))
{
}

void
CZMComputeLocalTractionIncrementalBase::computeInterfaceTractionAndDerivatives()
{
  _interface_displacement_jump_inc[_qp] =
      _interface_displacement_jump[_qp] - _interface_displacement_jump_old[_qp];
  computeInterfaceTractionIncrementAndDerivatives();
  _interface_traction[_qp] = _interface_traction_inc[_qp] + _interface_traction_old[_qp];
}
