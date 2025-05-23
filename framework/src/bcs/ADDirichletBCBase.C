//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBCBase.h"

InputParameters
ADDirichletBCBase::validParams()
{
  InputParameters params = NodalBCBase::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

ADDirichletBCBase::ADDirichletBCBase(const InputParameters & parameters)
  : NodalBCBase(parameters), _preset(getParam<bool>("preset"))
{
}
