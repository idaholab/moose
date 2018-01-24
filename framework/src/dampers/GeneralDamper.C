//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralDamper.h"

template <>
InputParameters
validParams<GeneralDamper>()
{
  InputParameters params = validParams<Damper>();
  return params;
}

GeneralDamper::GeneralDamper(const InputParameters & parameters) : Damper(parameters) {}
