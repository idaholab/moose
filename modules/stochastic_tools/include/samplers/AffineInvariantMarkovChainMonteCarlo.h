//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelMarkovChainMonteCarloBase.h"

/**
 * A class used to perform Affine Invariant Ensemble MCMC sampling
 */
class AffineInvariantMarkovChainMonteCarlo : public ParallelMarkovChainMonteCarloBase
{
public:
  static InputParameters validParams();

  AffineInvariantMarkovChainMonteCarlo(const InputParameters & parameters);

protected:
//   void initializeWalkers(const int & step);  
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// The step size for the affine invariant sampler
  const Real & _step_size;

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;

};
