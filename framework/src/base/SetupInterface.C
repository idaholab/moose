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

template<>
InputParameters validParams<SetupInterface>()
{
  InputParameters params = emptyInputParameters();

  // Get an MooseEnum of the available 'execute_on' options
  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());

  // Add the 'execute_on' input parameter for users to set
  params.addParam<MultiMooseEnum>("execute_on", execute_options, "Set to (nonlinear|linear|timestep_end|timestep_begin|custom) to execute only at that moment");

  return params;
}

SetupInterface::SetupInterface(const InputParameters & params)
{
  /*
   * While many of the MOOSE systems inherit from this interface, it doesn't make sense for them all to adjust their execution flags.
   * Our way of dealing with this is by not having those particular classes add the this classes valid params to their own.  In
   * those cases it won't exist so we just set it to a default and ignore it.
   */
  if (params.have_parameter<MultiMooseEnum>("execute_on"))
  {

    // Handle deprecated syntax
    MultiMooseEnum flags = params.get<MultiMooseEnum>("execute_on");
    std::map<std::string, std::string> syntax_conversion;
    syntax_conversion["residual"] = "linear";
    syntax_conversion["jacobian"] = "nonlinear";
    syntax_conversion["timestep"] = "timestep_end";

    for (std::map<std::string, std::string>::const_iterator it = syntax_conversion.begin(); it != syntax_conversion.end(); ++it)
      if (flags.contains(it->first))
      {
        mooseWarning("The 'execute_on' option '" << it->first << "' is deprecated, please replace with '" << it->second << "'.");
        flags.erase(it->first);
        flags.push_back(it->second);
      }

    // Set the execution flags for this object
    _exec_flags = Moose::vectorStringsToEnum<ExecFlagType>(flags);
  }

  else
    _exec_flags.push_back(EXEC_LINEAR);
}

SetupInterface::~SetupInterface()
{
}

void
SetupInterface::initialSetup() {}

void
SetupInterface::timestepSetup() {}

void
SetupInterface::jacobianSetup() {}

void
SetupInterface::residualSetup() {}

void
SetupInterface::subdomainSetup() {}

const std::vector<ExecFlagType> &
SetupInterface::execFlags() const
{
  return _exec_flags;
}

ExecFlagType
SetupInterface::execBitFlags() const
{
  unsigned int exec_bit_field = EXEC_NONE;
  for (unsigned int i=0; i<_exec_flags.size(); ++i)
    exec_bit_field |= _exec_flags[i];

  return static_cast<ExecFlagType>(exec_bit_field);
}

MultiMooseEnum
SetupInterface::getExecuteOptions()
{
  return MultiMooseEnum("initial=0x01 linear=0x02 nonlinear=0x04 timestep_end=0x08 timestep_begin=0x10 custom=0x100 residual=0x200 jacobian=0x400 timestep=0x800", "linear");
}
