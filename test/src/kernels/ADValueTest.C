//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADValueTest.h"

registerMooseObject("MooseTestApp", ADValueTest);

InputParameters
ADValueTest::validParams()
{
  InputParameters params = ADKernel::validParams();
  return params;
}

ADValueTest::ADValueTest(const InputParameters & parameters) : ADKernel(parameters) {}

ADReal
ADValueTest::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}
