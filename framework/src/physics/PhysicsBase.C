//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsBase.h"
#include "MooseUtils.h"

#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"

InputParameters
PhysicsBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Creates all the objects necessary to solve a particular physics");

  MooseEnum transient_options("true false same_as_problem", "same_as_problem");
  params.addParam<MooseEnum>(
      "transient", transient_options, "Whether the physics is to be solved as a transient");

  return params;
}

PhysicsBase::PhysicsBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    BlockRestrictable(this),
    _is_transient(getParam<MooseEnum>("transient"))
{
  _problem = getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  _factory = &_app.getFactory();

  if (_is_transient == "true" && !getProblem().isTransient())
    paramError("transient", "We cannot solve a physics as transient in a steady problem");
}

bool
PhysicsBase::isTransient() const
{
  if (_is_transient == "true")
    return true;
  else if (_is_transient == "false")
    return false;
  else
    return getProblem().isTransient();
}

void
PhysicsBase::checkParamsBothSetOrNotSet(std::string param1, std::string param2) const
{
  if ((isParamValid(param1) + isParamValid(param2)) % 2 != 0)
    paramError(param1,
               "Parameters " + param1 + " and " + param2 +
                   " must be either both set or both unset");
}

bool
PhysicsBase::nonLinearVariableExists(const VariableName & var_name, bool error_if_aux) const
{
  if (_problem->getNonlinearSystemBase().hasVariable(var_name))
    return true;
  else if (error_if_aux && _problem->getAuxiliarySystem().hasVariable(var_name))
    mooseError("Variable",
               var_name,
               "is supposed to be nonlinear for physics",
               name(),
               "but it's already defined as auxiliary");
  else
    return false;
}
