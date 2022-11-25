//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"
#include "ReporterInterface.h"
#include "LikelihoodInterface.h"

/**
 * A base class used to perform Parallel Markov Chain Monte Carlo (MCMC) sampling
 */
class ParallelMarkovChainMonteCarloBase : public Sampler, public ReporterInterface, public LikelihoodInterface
{
public:
  static InputParameters validParams();

  ParallelMarkovChainMonteCarloBase(const InputParameters & parameters);

  /**
   * Return the number of configuration parameters.
   */
  dof_id_type getNumberOfConfigParams() const;

  /**
   * Return the number of parallel proposals.
   */
  dof_id_type getNumParallelProposals() const;

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Reporter value the seed input values for proposing the next set of samples
  const std::vector<Real> & _seed_inputs;

  /// Reporter value for the proposal stds
  const std::vector<Real> & _proposal_std;

  /// Number of parallel proposals to be made and subApps to be executed
  const unsigned int & _num_parallel_proposals;

  /// Storage for the likelihood object to be utilized
  // const Likelihood & _likelihood;

  /// Initial values of the input params to get the MCMC scheme started
  const std::vector<Real> & _initial_values;

  /// Storage for prior distribution objects to be utilized
  std::vector<const Distribution *> _priors;

  /// Track the current step of the main App
  const int & _step;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int & _num_random_seeds;

  /// Configuration values
  std::vector<Real> _confg_values;

private:

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  std::vector<std::vector<Real>> _new_samples;

  std::vector<Real> _std_use;

  std::vector<Real> _lb;

  std::vector<Real> _ub;
  
};
