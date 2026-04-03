//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingClosureBase.h"
#include "SCM.h"

InputParameters
SCMMixingClosureBase::validParams()
{
  InputParameters params = SCMClosureBase::validParams();
  params.addParam<Real>("CT", 1.0, "Turbulent momentum, modeling parameter [-].");
  return params;
}

SCMMixingClosureBase::SCMMixingClosureBase(const InputParameters & parameters)
  : SCMClosureBase(parameters), _CT(getParam<Real>("CT"))
{
}
