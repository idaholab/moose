/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SetupInterface.h"
#include "Conversion.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<SetupInterface>()
{
  InputParameters params = emptyInputParameters();

  // Add the 'execute_on' input parameter for users to set
  MultiMooseEnum execute_options(MooseUtils::createExecuteOnEnum({EXEC_LINEAR}));
  params.addParam<MultiMooseEnum>("execute_on",
                                  execute_options,
                                  MooseUtils::getExecuteOnEnumDocString(execute_options));
  return params;
}

SetupInterface::SetupInterface(const MooseObject * moose_object)
  : _execute_enum(moose_object->parameters().isParamValid("execute_on")
                      ? moose_object->parameters().get<MultiMooseEnum>("execute_on")
                      : _empty_execute_enum),
    _exec_flags(_execute_enum.getCurrentIDs()),
    _current_execute_flag(
        (moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
            ->getCurrentExecuteOnFlag())
{
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

const MultiMooseEnum &
SetupInterface::getExecuteOnEnum() const
{
  return _execute_enum;
}

const std::vector<ExecFlagType> &
SetupInterface::execFlags() const
{
  mooseDeprecated("MOOSE has been updated to use a MultiMooseEnum for execute flags. The current "
                  "flags should be retrieved from the \"exeucte_on\" parameters of your object, "
                  "or by using the \"_execute_enum\" reference to the parameter or the "
                  "getExecuteOnEnum() method.");
  return _exec_flags;
}


ExecFlagType
SetupInterface::execBitFlags() const
{
  mooseDeprecated("This method has been removed because MOOSE was updated to use a MultiMooseEnum "
                  "for execute flags. This method does nothing so will likely alter application "
                  "execution.");
  return EXEC_NONE;
}

MultiMooseEnum
SetupInterface::getExecuteOptions()
{
  mooseDeprecated("The getExecuteOptions' was replaced by MooseUtils::createExecuteOnEnum because "
                  "MOOSE was updated to use a MultiMooseEnum for the execute flags and the "
                  "new function provides additional arguments for modification of the enum.");
  return MooseUtils::createExecuteOnEnum();
}
