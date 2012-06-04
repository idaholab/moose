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


SetupInterface::SetupInterface(InputParameters & params)
{
  /**
   * While many of the MOOSE systems inherit from this interface, it doesn't make sense for them all to adjust their execution flags.
   * Our way of dealing with this is by not having those particular classes add the this classes valid params to their own.  In
   * those cases it won't exist so we just set it to a default and ignore it.
   */
  if (params.have_parameter<std::string>("execute_on"))
    _exec_flags = Moose::stringToEnum<ExecFlagType>(params.get<std::string>("execute_on"));
  else
    _exec_flags = EXEC_RESIDUAL;   // ignored
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

ExecFlagType
SetupInterface::execFlag()
{
  return _exec_flags;
}

void
SetupInterface::execFlag(ExecFlagType type)
{
  _exec_flags = type;
}
