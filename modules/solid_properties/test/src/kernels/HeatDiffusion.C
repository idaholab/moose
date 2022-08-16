//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "HeatDiffusion.h"

registerMooseObject("SolidPropertiesTestApp", HeatDiffusion);

InputParameters
HeatDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Thermal conduction kernel");
  return params;
}

HeatDiffusion::HeatDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _k(getMaterialProperty<Real>("k_solid"))
{
}

Real
HeatDiffusion::computeQpResidual()
{
  return _k[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
HeatDiffusion::computeQpJacobian()
{
  // neglects contribution of derivatives in thermal conductivity
  return _k[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
