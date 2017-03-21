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

// STL includes
#include <vector>

// MOOSE includes
#include "MooseTypes.h"

namespace Moose
{

// Currently there are 7 exec types (See MooseTypes.h)
const std::vector<ExecFlagType>
populateExecTypes()
{
  std::vector<ExecFlagType> exec_types(7);
  exec_types[0] = EXEC_INITIAL;
  exec_types[1] = EXEC_TIMESTEP_BEGIN;
  exec_types[2] = EXEC_NONLINEAR;
  exec_types[3] = EXEC_LINEAR;
  exec_types[4] = EXEC_TIMESTEP_END;
  exec_types[5] = EXEC_CUSTOM;
  exec_types[6] = EXEC_SUBDOMAIN;
  return exec_types;
}

const std::vector<ExecFlagType> exec_types = populateExecTypes();

} // namespace Moose
