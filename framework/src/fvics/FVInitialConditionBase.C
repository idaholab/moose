//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInitialConditionBase.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "UserObject.h"

InputParameters
FVInitialConditionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += InitialConditionInterface::validParams();
  params += BlockRestrictable::validParams();

  params.addRequiredParam<VariableName>("variable",
                                        "The variable this initial condition is "
                                        "supposed to provide values for.");
  params.registerBase("FVInitialCondition");

  return params;
}

FVInitialConditionBase::FVInitialConditionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    InitialConditionInterface(parameters),
    BlockRestrictable(this),
    FunctionInterface(this),
    Restartable(this, "FVInitialConditionBases"),
    DependencyResolverInterface(),
    NonADFunctorInterface(this),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _depend_vars(std::set<std::string>())
{
  _supplied_vars.insert(getParam<VariableName>("variable"));
}

FVInitialConditionBase::~FVInitialConditionBase() {}
