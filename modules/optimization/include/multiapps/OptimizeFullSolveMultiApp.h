//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// fixme lynn this is included because MultiApp probably needs it
#include "libmesh/numeric_vector.h"

// MOOSE includes
#include "FullSolveMultiApp.h"

/**
 * This is FullSolveMultiApp with some extra flags registered.
 */
class OptimizeFullSolveMultiApp : public FullSolveMultiApp
{
public:
  static InputParameters validParams();
  OptimizeFullSolveMultiApp(const InputParameters & parameters);

  virtual void preTransfer(Real dt, Real target_time) override;
};
