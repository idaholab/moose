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
  dof_id_type getNumberOfConfigParams() const{ return  _confg_values.size(); }

  /**
   * Return the number of parallel proposals.
   */
  dof_id_type getNumParallelProposals() const{ return _num_parallel_proposals; }

  /**
   * Return the random numbers to facilitate decision making in reporters
   */
  std::vector<Real> getRandomNumbers() const { return _rnd_vec; }

  /**
   * Return the step after which decision making can begin
   */
  virtual int decisionStep() const { return 1; }

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;

  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Sample a random index excluding a specified index
  void randomIndex(const unsigned int & ub, const unsigned int & exclude, const unsigned int & seed, unsigned int & req_index);

  /// Sample two random indices without repitition excluding a specified index
  void randomIndex2(const unsigned int & ub, const unsigned int & exclude, const unsigned int & seed, unsigned int & req_index1, unsigned int & req_index2);

  /// Number of parallel proposals to be made and subApps to be executed
  const unsigned int & _num_parallel_proposals;

  /// Storage for prior distribution objects to be utilized
  std::vector<const Distribution *> _priors;

  /// Lower bounds for making the next proposal
  const std::vector<Real> * _lb;

  /// Upper bounds for making the next proposal
  const std::vector<Real> * _ub;

  /// Track the current step of the main App
  const int & _step;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Vectors of new proposed samples
  std::vector<std::vector<Real>> _new_samples;

  /// Vector of random numbers for decision making
  std::vector<Real> _rnd_vec;

private:
  /**
   * Generates combinations of the new samples with the experimental configurations
   */
  void combineWithConfg();

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int & _num_random_seeds;

  /// Configuration values
  std::vector<Real> _confg_values;

  /// Vectors of new proposed samples combined with the experimental configuration values
  std::vector<std::vector<Real>> _new_samples_confg;
};
