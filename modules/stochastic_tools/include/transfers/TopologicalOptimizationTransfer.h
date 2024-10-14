//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsTransfer.h"

class TopologyOptimizerDecision;

/**
 * This class transfers the configuration from the topology reporter to the
 * multiapp system and updates the mesh.
 */
class TopologicalOptimizationTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();
  TopologicalOptimizationTransfer(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialSetup() override;

protected:
  /// the reporter that makes decision on accepting or rejecting proposals
  TopologyOptimizerDecision * _reporter;

  /// the name of the postprocessor on the subapp that stores the objective function value
  PostprocessorName _objective_name;
};
