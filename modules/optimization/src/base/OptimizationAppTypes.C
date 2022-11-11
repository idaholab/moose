//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationAppTypes.h"
#include "ExecFlagRegistry.h"

namespace OptimizationAppTypes
{
const ExecFlagType EXEC_FORWARD = registerDefaultExecFlag("FORWARD");
const ExecFlagType EXEC_ADJOINT = registerDefaultExecFlag("ADJOINT");
const ExecFlagType EXEC_HOMOGENEOUS_FORWARD = registerDefaultExecFlag("HOMOGENEOUS_FORWARD");
}
