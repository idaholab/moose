//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMConstitutiveModelIncrementalBase.h"

InputParameters
CZMConstitutiveModelIncrementalBase::validParams()
{
  InputParameters params = CZMConstitutiveModelTotalBase::validParams();

  params.addClassDescription(
      "Base class for implementing incremental cohesive zone constituive material models");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMConstitutiveModelIncrementalBase::CZMConstitutiveModelIncrementalBase(
    const InputParameters & parameters)
  : CZMConstitutiveModelTotalBase(parameters),
    _interface_traction_inc(declareProperty<RealVectorValue>("interface_traction_inc")),
    _interface_traction_old(getMaterialPropertyOld<RealVectorValue>("interface_traction")),
    _interface_displacement_jump_inc(
        declareProperty<RealVectorValue>("interface_displacement_jump_inc")),
    _interface_displacement_jump_old(
        getMaterialPropertyOld<RealVectorValue>("interface_displacement_jump"))
{
}

void
CZMConstitutiveModelIncrementalBase::initQpStatefulProperties()
{
  _interface_traction[_qp] = 0;
}

void
CZMConstitutiveModelIncrementalBase::computeInterfaceTractionAndDerivatives()
{
  _interface_displacement_jump_inc[_qp] =
      _interface_displacement_jump[_qp] - _interface_displacement_jump_old[_qp];
  computeInterfaceTractionIncrementAndDerivatives();
  _interface_traction[_qp] = _interface_traction_inc[_qp] + _interface_traction_old[_qp];
}
