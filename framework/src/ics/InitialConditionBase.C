//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitialConditionBase.h"
#include "MooseEnum.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "UserObject.h"

InputParameters
InitialConditionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();
  params += MaterialPropertyInterface::validParams();

  params.addRequiredParam<VariableName>("variable",
                                        "The variable this initial condition is "
                                        "supposed to provide values for.");
  params.addParam<bool>("ignore_uo_dependency",
                        false,
                        "When set to true, a UserObject retrieved "
                        "by this IC will not be executed before the "
                        "this IC");
  params.addParamNamesToGroup("ignore_uo_dependency", "Advanced");

  MooseEnum stateEnum("CURRENT=0 OLD=1 OLDER=2", "CURRENT");
  params.addParam<MooseEnum>(
      "state",
      stateEnum,
      "This parameter is used to set old state solutions at the start of simulation. If specifying "
      "multiple states at the start of simulation, use one IC object for each state being "
      "specified. The states are CURRENT=0 OLD=1 OLDER=2. States older than 2 are not currently "
      "supported. When the user only specifies current state, the solution is copied to the old "
      "and older states, as expected. This functionality is mainly used for dynamic simulations "
      "with explicit time integration schemes, where old solution states are used in the velocity "
      "and acceleration approximations.");

  params.registerBase("InitialCondition");

  return params;
}

InitialConditionBase::InitialConditionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    Coupleable(this,
               getParam<SystemBase *>("_sys")
                   ->getVariable(parameters.get<THREAD_ID>("_tid"),
                                 parameters.get<VariableName>("variable"))
                   .isNodal()),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    BoundaryRestrictable(this, _c_nodal),
    DependencyResolverInterface(),
    Restartable(this, "InitialConditionBases"),
    ElementIDInterface(this),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _ignore_uo_dependency(getParam<bool>("ignore_uo_dependency")),
    _my_state(getParam<MooseEnum>("state"))
{
  _supplied_vars.insert(getParam<VariableName>("variable"));

  const auto & coupled_vars = getCoupledVars();
  for (const auto & it : coupled_vars)
    for (const auto & var : it.second)
      _depend_vars.insert(var->name());
}

InitialConditionBase::~InitialConditionBase() {}

const std::set<std::string> &
InitialConditionBase::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
InitialConditionBase::getSuppliedItems()
{
  return _supplied_vars;
}

void
InitialConditionBase::addUserObjectDependencyHelper(const UserObject & uo) const
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(uo.name());
}

void
InitialConditionBase::addPostprocessorDependencyHelper(const PostprocessorName & name) const
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(name);
}

int &
InitialConditionBase::getMyState()
{
  return _my_state;
}