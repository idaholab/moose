//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Diffusion.h"

registerMooseObject("MooseApp", Diffusion);

InputParameters
Diffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("The diffusion term ($-\\nabla \\cdot k\\nabla u$), with the "
                             "weak form of $(\\nabla \\phi_i, k\\nabla u_h)$.");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  params.declareControllable("coeff");
  return params;
}

Diffusion::Diffusion(const InputParameters & parameters)
  : Kernel(parameters), _coeff(getParam<Real>("coeff"))
{
}

Real
Diffusion::computeQpResidual()
{
  return _coeff * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
Diffusion::computeQpJacobian()
{
  return _coeff * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
