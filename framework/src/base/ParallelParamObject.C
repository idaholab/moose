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

ParallelParamObject::ParallelParamObject(const std::string & type,
                                         const std::string & name,
                                         MooseApp & app,
                                         const InputParameters & params)
  : MooseBase(type, name, app, params),
    MooseBaseParameterInterface(*this, params),
    MooseBaseErrorInterface(static_cast<MooseBase &>(*this)),
    ParallelObject(app),
    DataFileInterface(*this)
{
}
