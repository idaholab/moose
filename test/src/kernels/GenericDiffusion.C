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
#include "GenericDiffusion.h"

template <>
InputParameters
validParams<GenericDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>(
      "property", "The name of the material property to use as the diffusivity.");
  return params;
}

GenericDiffusion::GenericDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _diffusivity(getMaterialProperty<Real>(parameters.get<std::string>("property")))
{
}

Real
GenericDiffusion::computeQpResidual()
{
  return _diffusivity[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
GenericDiffusion::computeQpJacobian()
{
  return _diffusivity[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
