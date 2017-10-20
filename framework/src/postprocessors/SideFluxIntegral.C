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

#include "SideFluxIntegral.h"

template <>
InputParameters
validParams<SideFluxIntegral>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  params.addClassDescription("Computes the integral of the flux over the specified boundary");
  return params;
}

SideFluxIntegral::SideFluxIntegral(const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),
    _diffusivity(parameters.get<MaterialPropertyName>("diffusivity")),
    _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{
}

Real
SideFluxIntegral::computeQpIntegral()
{
  return -_diffusion_coef[_qp] * _grad_u[_qp] * _normals[_qp];
}
