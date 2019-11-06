//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledEigenKernel.h"

registerMooseObject("MooseTestApp", CoupledEigenKernel);

InputParameters
CoupledEigenKernel::validParams()
{
  InputParameters params = EigenKernel::validParams();
  params.addRequiredCoupledVar("v", "Variable to be coupled in");
  return params;
}

CoupledEigenKernel::CoupledEigenKernel(const InputParameters & parameters)
  : EigenKernel(parameters), _v(coupledValue("v"))
{
}

Real
CoupledEigenKernel::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}
