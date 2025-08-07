//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelParamObject.h"

#include "MooseApp.h"

InputParameters
ParallelParamObject::validParams()
{
  return MooseBase::validParams();
}

ParallelParamObject::ParallelParamObject(const InputParameters & params)
  : MooseBase(params),
    ParallelObject(_app),
    DataFileInterface(*this),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory())
{
}
