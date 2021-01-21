//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVScalarFieldVariable.h"

registerMooseObject("NavierStokesApp", INSFVScalarFieldVariable);

InputParameters
INSFVScalarFieldVariable::validParams()
{
  return INSFVVariable::validParams();
}

INSFVScalarFieldVariable::INSFVScalarFieldVariable(const InputParameters & params)
  : INSFVVariable(params)
{
}
