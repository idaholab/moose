//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SelfAux.h"

registerMooseObject("MooseApp", SelfAux);

defineLegacyParams(SelfAux);

InputParameters
SelfAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  return params;
}

SelfAux::SelfAux(const InputParameters & parameters) : AuxKernel(parameters) {}

Real
SelfAux::computeValue()
{
  return _u[_qp];
}
