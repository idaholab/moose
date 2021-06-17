//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoeffParamDiffusion.h"

registerMooseObject("MooseTestApp", CoeffParamDiffusion);

// MooseDocs::start
InputParameters
CoeffParamDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addRequiredParam<Real>("D", "The diffusivity coefficient.");
  return params;
}
// MooseDocs::end

CoeffParamDiffusion::CoeffParamDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),

    _D(getParam<Real>("D"))
{
}

CoeffParamDiffusion::~CoeffParamDiffusion() {}

Real
CoeffParamDiffusion::computeQpResidual()
{
  return _D * Diffusion::computeQpResidual();
}

Real
CoeffParamDiffusion::computeQpJacobian()
{
  return _D * Diffusion::computeQpJacobian();
}
