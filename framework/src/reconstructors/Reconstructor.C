//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Reconstructor.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
Reconstructor::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("Reconstructor");
  return params;
}

Reconstructor::Reconstructor(const InputParameters & params)
  : MooseObject(params),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _tid(params.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid))
{
}
