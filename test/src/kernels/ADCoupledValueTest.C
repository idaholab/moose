//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledValueTest.h"

registerMooseObject("MooseTestApp", ADCoupledValueTest);

InputParameters
ADCoupledValueTest::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addCoupledVar("v", 2.0, "The coupled variable.");
  return params;
}

ADCoupledValueTest::ADCoupledValueTest(const InputParameters & parameters)
  : ADKernel(parameters), _v(adCoupledValue("v"))
{
}

ADReal
ADCoupledValueTest::computeQpResidual()
{
  return _test[_i][_qp] * -_v[_qp];
}
