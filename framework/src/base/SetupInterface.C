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
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = EXEC_LINEAR;
  params.addParam<ExecFlagEnum>("execute_on", execute_options, execute_options.getDocString());
  return params;
}

SetupInterface::SetupInterface(const MooseObject * moose_object)
  : _execute_enum(moose_object->parameters().isParamValid("execute_on")
                      ? moose_object->parameters().get<ExecFlagEnum>("execute_on")
                      : _empty_execute_enum),
    _exec_flags(_execute_enum.begin(), _execute_enum.end()), // deprecated TODO: ExecFlagType
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

const std::vector<ExecFlagType> &
SetupInterface::execFlags() const
{
  // TODO: ExecFlagType
  mooseDeprecated("The execFlags() method is being removed because MOOSE has been updated to use a "
                  "ExecFlagEnum for execute flags. The current flags should be retrieved from "
                  "the \"exeucte_on\" parameters of your object or by using the \"_execute_enum\" "
                  "reference to the parameter or the getExecuteOnEnum() method.");

  return _exec_flags;
}

ExecFlagType
SetupInterface::execBitFlags() const
{
  // TODO: ExecFlagType
  mooseDeprecated("The execBitFlags method is being removed because MOOSE was updated to use a "
                  "ExecFlagEnum for execute flags. This method maintains the behavior of the "
                  "original method but the use of this method should be removed from your "
                  "application. The ExecFlagEnum should be inspected directly via the "
                  "getExecuteOnEnum() method.");

  unsigned int exec_bit_field = EXEC_NONE;
  for (const auto & flag : _exec_flags)
    exec_bit_field |= flag.id();
  return ExecFlagType("deprecated", exec_bit_field);
}

ExecFlagEnum
SetupInterface::getExecuteOptions()
{
  // TODO: ExecFlagType
  ::mooseDeprecated("The 'getExecuteOptions' was replaced by the ExecFlagEnum class because MOOSE "
                    "was updated to use this for the execute flags and the new function provides "
                    "additional arguments for modification of the enum.");
  return MooseUtils::getDefaultExecFlagEnum();
}
