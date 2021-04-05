//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitODE.h"

registerMooseObject("MooseTestApp", ExplicitODE);

InputParameters
ExplicitODE::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addParam<Real>("lambda", 1, "Lambda on the right-hand side");
  return params;
}

ExplicitODE::ExplicitODE(const InputParameters & parameters)
  : AuxScalarKernel(parameters), _u_old(uOld()), _lambda(getParam<Real>("lambda"))
{
}

ExplicitODE::~ExplicitODE() {}

Real
ExplicitODE::computeValue()
{
  return _u_old[_i] * (1 - (_lambda * _dt));
}
