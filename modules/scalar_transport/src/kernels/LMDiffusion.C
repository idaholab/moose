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
  params.addRequiredCoupledVar("primal_variable",
                               "The coupled primal variable from which to pull the Laplacian");
  params.addParam<bool>(
      "lm_sign_positive",
      true,
      "Whether to use a positive sign when adding this object's residual to the Lagrange "
      "multiplier constraint equation. Positive or negative sign should be chosen such that the "
      "diagonals for the LM block of the matrix are positive");
  params.addParam<Real>("diffusivity", 1, "The value of the diffusivity");
  params.addClassDescription(
      "Adds a diffusion term to a Lagrange multiplier constrained primal equation");
  return params;
}

LMDiffusion::LMDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _primal_var(coupled("primal_variable")),
    _second_primal(coupledSecond("primal_variable")),
    _second_primal_phi(getVar("primal_variable", 0)->secondPhi()),
    _lm_sign(getParam<bool>("lm_sign_positive") ? 1. : -1),
    _diffusivity(getParam<Real>("diffusivity"))
{
  if (_var.number() == _primal_var)
    paramError(
        "primal_variable",
        "Coupled variable 'primal_variable' needs to be different from 'variable' with "
        "LMDiffusion. It is expected in general that 'variable' should be a Lagrange multiplier "
        "variable, and that 'primal_variable' be the primal variable on which the Lagrange "
        "multiplier is acting");
}

Real
LMDiffusion::computeQpResidual()
{
  return _lm_sign * _test[_i][_qp] * -_diffusivity * _second_primal[_qp].tr();
}

Real
LMDiffusion::computeQpJacobian()
{
  return 0;
}

Real
LMDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _primal_var)
    return _lm_sign * _test[_i][_qp] * -_diffusivity * _second_primal_phi[_j][_qp].tr();

  return 0;
}
