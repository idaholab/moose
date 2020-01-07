//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericDiffusion.h"

registerMooseObject("MooseTestApp", GenericDiffusion);

InputParameters
GenericDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
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
