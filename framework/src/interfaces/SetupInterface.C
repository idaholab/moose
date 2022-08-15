//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupInterface.h"
#include "Conversion.h"
#include "FEProblem.h"

InputParameters
SetupInterface::validParams()
{

  InputParameters params = emptyInputParameters();

  // Add the 'execute_on' input parameter for users to set
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = EXEC_LINEAR;
  params.addParam<ExecFlagEnum>("execute_on", execute_options, execute_options.getDocString());
  return params;
}

SetupInterface::SetupInterface(const MooseObject * moose_object)
  : _execute_enum(moose_object->parameters().isParamValid("execute_on")
                      ? moose_object->parameters().get<ExecFlagEnum>("execute_on")
                      : _empty_execute_enum),
    _current_execute_flag(
        (moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
            ->getCurrentExecuteOnFlag())
{
  _empty_execute_enum.clear(); // remove any flags for the case when "execute_on" is not used
}

SetupInterface::~SetupInterface() {}

void
SetupInterface::initialSetup()
{
}

void
SetupInterface::timestepSetup()
{
}

void
SetupInterface::jacobianSetup()
{
}

void
SetupInterface::residualSetup()
{
}

void
SetupInterface::subdomainSetup()
{
}

const ExecFlagEnum &
SetupInterface::getExecuteOnEnum() const
{
  return _execute_enum;
}
