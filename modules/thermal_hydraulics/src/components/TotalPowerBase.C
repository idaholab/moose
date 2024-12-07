//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalPowerBase.h"

InputParameters
TotalPowerBase::validParams()
{
  InputParameters params = Component::validParams();
  return params;
}

TotalPowerBase::TotalPowerBase(const InputParameters & parameters)
  : Component(parameters), _power_var_name(genName(name(), "power"))
{
}

void
TotalPowerBase::addVariables()
{
  getTHMProblem().addSimVariable(false, _power_var_name, libMesh::FEType(FIRST, SCALAR));
}
