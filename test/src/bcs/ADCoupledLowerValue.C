//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledLowerValue.h"

registerMooseObject("MooseTestApp", ADCoupledLowerValue);

InputParameters
ADCoupledLowerValue::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredCoupledVar("lower_d_var", "the lower dimensional variable to couple in");
  return params;
}

ADCoupledLowerValue::ADCoupledLowerValue(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _lower(adCoupledLowerValue("lower_d_var"))
{
}

ADReal
ADCoupledLowerValue::computeQpResidual()
{
  return _test[_i][_qp] * -_lower[_qp];
}
