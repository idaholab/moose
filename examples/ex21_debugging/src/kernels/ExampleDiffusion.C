//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleDiffusion.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
registerMooseObject("ExampleApp", ExampleDiffusion);

InputParameters
ExampleDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addRequiredCoupledVar(
      "coupled_coef", "The value of this variable will be used as the diffusion coefficient.");

  return params;
}

ExampleDiffusion::ExampleDiffusion(const InputParameters & parameters)
  : Diffusion(parameters), _coupled_coef(coupledValue("coupled_coef"))
{
}

Real
ExampleDiffusion::computeQpResidual()
{
  return _coupled_coef[_qp] * Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  return _coupled_coef[_qp] * Diffusion::computeQpJacobian();
}
