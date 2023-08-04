//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TopologicalConstraintBase.h"

InputParameters
TopologicalConstraintBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for topological optimization constraints. Override "
                             "isConfigAllowed to implement custom constraint.");
  return params;
}

TopologicalConstraintBase::TopologicalConstraintBase(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}
