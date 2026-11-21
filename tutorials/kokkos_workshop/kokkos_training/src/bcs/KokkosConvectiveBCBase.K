//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectiveBCBase.h"

InputParameters
KokkosConvectiveBCBase::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addRequiredParam<Real>("u_inf", "Far-field value");
  return params;
}

KokkosConvectiveBCBase::KokkosConvectiveBCBase(const InputParameters & parameters)
  : IntegratedBCValue(parameters), _u_inf(getParam<Real>("u_inf"))
{
}
