//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Level set includes
#include "LevelSetTypes.h"

// MOOSE includes
#include "ExecFlagRegistry.h"

const ExecFlagType LevelSet::EXEC_ADAPT_MESH = registerExecFlag("ADAPT_MESH");
const ExecFlagType LevelSet::EXEC_COMPUTE_MARKERS = registerExecFlag("COMPUTE_MARKERS");
