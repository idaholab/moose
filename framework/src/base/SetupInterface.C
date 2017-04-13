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

  // Get an MooseEnum of the available 'execute_on' options
  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());

  // Add the 'execute_on' input parameter for users to set
  params.addParam<MultiMooseEnum>("execute_on",
                                  execute_options,
                                  "Set to (nonlinear|linear|timestep_end|timestep_begin|custom) "
                                  "to execute only at that moment");

  // The Output system uses different options for the 'execute_on' than other systems, therefore the
  // check of the options
  // cannot occur based on the 'execute_on' parameter, so this flag triggers the check
  params.addPrivateParam<bool>("check_execute_on", true);

  return params;
}

SetupInterface::SetupInterface(const MooseObject * moose_object)
  : _current_execute_flag(
        (moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
            ->getCurrentExecuteOnFlag())
{
  const InputParameters & params = moose_object->parameters();

  /**
   * While many of the MOOSE systems inherit from this interface, it doesn't make sense for them all
   * to adjust their execution flags.
   * Our way of dealing with this is by not having those particular classes add the this classes
   * valid params to their own.  In
   * those cases it won't exist so we just set it to a default and ignore it.
   */
  if (params.have_parameter<bool>("check_execute_on") && params.get<bool>("check_execute_on"))
  {
    MultiMooseEnum flags = params.get<MultiMooseEnum>("execute_on");
    _exec_flags = Moose::vectorStringsToEnum<ExecFlagType>(flags);
  }

  else
    _exec_flags.push_back(EXEC_LINEAR);
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

const std::vector<ExecFlagType> &
SetupInterface::execFlags() const
{
  return _exec_flags;
}

ExecFlagType
SetupInterface::execBitFlags() const
{
  unsigned int exec_bit_field = EXEC_NONE;
  for (unsigned int i = 0; i < _exec_flags.size(); ++i)
    exec_bit_field |= _exec_flags[i];

  return static_cast<ExecFlagType>(exec_bit_field);
}

MultiMooseEnum
SetupInterface::getExecuteOptions()
{
  return MultiMooseEnum("none=0x00 initial=0x01 linear=0x02 nonlinear=0x04 timestep_end=0x08 "
                        "timestep_begin=0x10 custom=0x100",
                        "linear");
}
