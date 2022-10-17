//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMDiffusion.h"

registerMooseObject("ScalarTransportApp", LMDiffusion);

InputParameters
LMDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable from which to pull the Laplacian");
  params.addParam<Real>("lm_sign", 1, "The sign of the lagrange multiplier in the primal equation");
  params.addParam<Real>("diffusivity", 1, "The value of the diffusivity");
  params.addClassDescription(
      "Adds a diffusion term to a Lagrange multiplier constrained primal equation");
  return params;
}

LMDiffusion::LMDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _second_v(coupledSecond("v")),
    _second_v_phi(getVar("v", 0)->secondPhi()),
    _lm_sign(getParam<Real>("lm_sign")),
    _diffusivity(getParam<Real>("diffusivity"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with "
               "LMDiffusion");
}

Real
LMDiffusion::computeQpResidual()
{
  return _lm_sign * _test[_i][_qp] * -_diffusivity * _second_v[_qp].tr();
}

Real
LMDiffusion::computeQpJacobian()
{
  return 0;
}

Real
LMDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _lm_sign * _test[_i][_qp] * -_diffusivity * _second_v_phi[_j][_qp].tr();

  return 0;
}
