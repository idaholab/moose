//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "StochasticToolsTransfer.h"

// Forward declarations
class TopologyOptimizationSampler;

/**
 * This transfer initializes the TopologyOptimizationSampler
 * with the initial guess of the geometry of the physics problem
 */
class InitialTopologicalMeshTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();

  InitialTopologicalMeshTransfer(const InputParameters & parameters);
  virtual void execute() override;

protected:
  TopologyOptimizationSampler * _top_opt_sampler;
};
