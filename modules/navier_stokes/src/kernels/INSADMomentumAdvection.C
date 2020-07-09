//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumAdvection.h"

registerMooseObject("NavierStokesApp", INSADMomentumAdvection);

InputParameters
INSADMomentumAdvection::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params.addClassDescription("Adds the advective term to the INS momentum equation");
  return params;
}

INSADMomentumAdvection::INSADMomentumAdvection(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _advective_strong_residual(getADMaterialProperty<RealVectorValue>("advective_strong_residual"))
{
}

ADRealVectorValue
INSADMomentumAdvection::precomputeQpResidual()
{
  return _advective_strong_residual[_qp];
}
