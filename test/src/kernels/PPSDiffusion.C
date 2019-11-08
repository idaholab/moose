//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PPSDiffusion.h"

registerMooseObject("MooseTestApp", PPSDiffusion);

InputParameters
PPSDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<PostprocessorName>("pps_name",
                                             "the name of the postprocessor we are going to use");
  return params;
}

PPSDiffusion::PPSDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _pps_value(getPostprocessorValue("pps_name"))
{
}

Real
PPSDiffusion::computeQpResidual()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
PPSDiffusion::computeQpJacobian()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
