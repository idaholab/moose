//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrBase.h"

template <>
InputParameters
validParams<ACGrGrBase>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addRequiredCoupledVar("v",
                               "Array of coupled order paramter names for other order parameters");
  return params;
}

ACGrGrBase::ACGrGrBase(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _vals_var(_op_num),
    _mu(getMaterialProperty<Real>("mu"))
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}
