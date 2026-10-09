//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearTimeFunction.h"

registerKokkosFunction("HeatTransferTestApp", KokkosLinearTimeFunction);

InputParameters
KokkosLinearTimeFunction::validParams()
{
  InputParameters params = FunctionBase::validParams();
  params.addClassDescription("Test function a + b * t with its time derivative.");
  params.addRequiredParam<Real>("a", "Constant term");
  params.addRequiredParam<Real>("b", "Linear coefficient");
  return params;
}

KokkosLinearTimeFunction::KokkosLinearTimeFunction(const InputParameters & parameters)
  : FunctionBase(parameters), _a(getParam<Real>("a")), _b(getParam<Real>("b"))
{
}
