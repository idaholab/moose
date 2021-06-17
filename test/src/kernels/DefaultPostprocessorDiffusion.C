//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultPostprocessorDiffusion.h"

registerMooseObject("MooseTestApp", DefaultPostprocessorDiffusion);

InputParameters
DefaultPostprocessorDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<PostprocessorName>(
      "pps_name",
      0.1,
      "The name of the postprocessor we are going to use, if the name is not "
      "found a default value of 0.1 is utlized for the postprocessor value");
  return params;
}

DefaultPostprocessorDiffusion::DefaultPostprocessorDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _pps_value(getPostprocessorValue("pps_name"))
{
}

Real
DefaultPostprocessorDiffusion::computeQpResidual()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
DefaultPostprocessorDiffusion::computeQpJacobian()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
