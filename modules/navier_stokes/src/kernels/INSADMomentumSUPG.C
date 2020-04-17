//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumSUPG.h"

registerMooseObject("NavierStokesApp", INSADMomentumSUPG);

InputParameters
INSADMomentumSUPG::validParams()
{
  InputParameters params = ADVectorKernelSUPG::validParams();
  params.addClassDescription("Adds the supg stabilization to the INS momentum equation");
  return params;
}

INSADMomentumSUPG::INSADMomentumSUPG(const InputParameters & parameters)
  : ADVectorKernelSUPG(parameters),
    _momentum_strong_residual(getADMaterialProperty<RealVectorValue>("momentum_strong_residual"))
{
}

ADRealVectorValue
INSADMomentumSUPG::precomputeQpStrongResidual()
{
  return _momentum_strong_residual[_qp];
}
