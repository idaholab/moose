//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumForces.h"

registerMooseObject("NavierStokesApp", INSADMomentumForces);

InputParameters
INSADMomentumForces::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params.addClassDescription("Adds body forces to the INS momentum equation");
  return params;
}

INSADMomentumForces::INSADMomentumForces(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _gravity_strong_residual(getADMaterialProperty<RealVectorValue>("gravity_strong_residual")),
    _mms_function_strong_residual(
        getADMaterialProperty<RealVectorValue>("mms_function_strong_residual"))
{
}

ADRealVectorValue
INSADMomentumForces::precomputeQpResidual()
{
  return _gravity_strong_residual[_qp] + _mms_function_strong_residual[_qp];
}
