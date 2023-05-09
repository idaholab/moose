//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMComputeLocalTractionIncrementalBase.h"
#include "CZMComputeLocalTractionIncrementalBase.h"

InputParameters
ADCZMComputeLocalTractionIncrementalBase::validParams()
{
  InputParameters params = CZMComputeLocalTractionIncrementalBase::validParams();
  return params;
}

ADCZMComputeLocalTractionIncrementalBase::ADCZMComputeLocalTractionIncrementalBase(
    const InputParameters & parameters)
  : ADCZMComputeLocalTractionBase(parameters),
    _interface_traction_inc(
        declareADPropertyByName<RealVectorValue>(_base_name + "interface_traction_inc")),
    _interface_traction_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_traction")),
    _interface_displacement_jump_inc(
        declareADPropertyByName<RealVectorValue>(_base_name + "interface_displacement_jump_inc")),
    _interface_displacement_jump_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_displacement_jump"))
{
}

void
ADCZMComputeLocalTractionIncrementalBase::computeInterfaceTraction()
{
  _interface_displacement_jump_inc[_qp] =
      _interface_displacement_jump[_qp] - _interface_displacement_jump_old[_qp];
  computeInterfaceTractionIncrement();
  _interface_traction[_qp] = _interface_traction_inc[_qp] + _interface_traction_old[_qp];
}
