//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchRLTrajectoryBuffer.h"

#include <vector>

/**
 * Computes GAE advantages and value targets for an on-policy trajectory buffer.
 */
class LibtorchRLValueEstimator
{
public:
  struct Targets
  {
    std::vector<Real> advantages;
    std::vector<Real> value_targets;
  };

  LibtorchRLValueEstimator(Real discount_factor, Real lambda_factor);

  void computeValueTargets(LibtorchRLTrajectoryBuffer & buffer,
                           Moose::LibtorchArtificialNeuralNet & value_network) const;

  Targets estimate(const LibtorchRLTrajectoryBuffer::Trajectory & trajectory,
                   Moose::LibtorchArtificialNeuralNet & value_network) const;

private:
  std::vector<Real> evaluate(const std::vector<std::vector<Real>> & observations,
                             Moose::LibtorchArtificialNeuralNet & value_network) const;

  const Real _discount_factor;
  const Real _lambda_factor;
};

#endif
