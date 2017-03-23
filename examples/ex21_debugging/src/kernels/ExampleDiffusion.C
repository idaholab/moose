/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExampleDiffusion.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template <>
InputParameters
validParams<ExampleDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
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
