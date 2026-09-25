//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"
#include "TransientInterface.h"
#include "Distribution.h"

/**
 * A base class used to perform Parallel Markov Chain Monte Carlo (MCMC) sampling
 */
class PMCMCBase : public Sampler, public TransientInterface
{
public:
  static InputParameters validParams();

  PMCMCBase(const InputParameters & parameters);

  /**
   * Return the number of configuration parameters.
   */
  dof_id_type getNumberOfConfigValues() const { return _confg_values[0].size(); }

  /**
   * Return the number of configuration parameters.
   */
  dof_id_type getNumberOfConfigParams() const { return _confg_values.size(); }

  /**
   * Return the number of parallel proposals.
   */
  dof_id_type getNumParallelProposals() const { return _num_parallel_proposals; }

  /**
   * Return the random numbers to facilitate decision making in reporters
   */
  const std::vector<Real> & getRandomNumbers() const;

  /**
   * Return the proposed variance samples to facilitate decision making in reporters
   */
  const std::vector<Real> & getVarSamples() const;

  /**
   * Return the proposed samples to facilitate decision making in reporters.
   * In MCMC schemes, there is a decision-making step after evaluating the
   * computational model on whether or not to accept the proposed samples.
   * To facilitate this decision-making, which happens in the Reporter, we
   * have to provide it the proposed samples.
   */
  const std::vector<std::vector<Real>> & getSamples() const;

  /**
   * Return the priors to facilitate decision making in reporters
   */
  const std::vector<const Distribution *> getPriors() const;

  /**
   * Return the prior over variance to facilitate decision making in reporters
   */
  const Distribution * getVarPrior() const;

  /**
   * Return the step after which decision making can begin
   */
  virtual int decisionStep() const { return 1; }

protected:
  /**
   * Fill in the _new_samples vector of vectors (happens within sampleSetUp)
   * @param seed_value The seed for the random number generator
   */
  virtual void proposeSamples();

  // See Sampler.h for description
  virtual void executeSetUp() override;

  // See Sampler.h for description
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) const override;

  /**
   * Sample a random number between 0 and 1
   * @param upper_bound The upper bound provided
   * @return The required index
   */
  Real random();

  /**
   * Sample a random index excluding a specified index
   * @param upper_bound The upper bound provided
   * @return The required index
   */
  unsigned int randomIndex(const unsigned int & upper_bound, const unsigned int & exclude);

  /**
   * Sample two random indices without repitition excluding a specified index
   * @param upper_bound The upper bound provided
   * @param exclude The index to be excluded from sampling
   * @return Pair of required indices
   */
  std::pair<unsigned int, unsigned int> randomIndexPair(const unsigned int & upper_bound,
                                                        const unsigned int & exclude);

  /// Number of parallel proposals to be made and subApps to be executed
  const unsigned int _num_parallel_proposals;

  /// Storage for prior distribution objects to be utilized
  std::vector<const Distribution *> _priors;

  /// Storage for prior distribution object of the variance to be utilized
  const Distribution * _var_prior;

  /// Lower bounds for making the next proposal
  const std::vector<Real> * _lower_bound;

  /// Upper bounds for making the next proposal
  const std::vector<Real> * _upper_bound;

  /// Upper bound for variance for making the next proposal
  const Real & _variance_bound;

  /// Initial values of the input params to get the MCMC scheme started
  const std::vector<Real> & _initial_values;

  /// Vectors of new proposed samples
  std::vector<std::vector<Real>> _new_samples;

  /// Vector of new proposed variance samples
  std::vector<Real> & _new_var_samples;

  /// Vector of random numbers for decision making
  std::vector<Real> & _rnd_vec;

private:
  /**
   * Generates combinations of the new samples with the experimental configurations
   */
  void combineWithExperimentalConfig();

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int _num_random_seeds;

  /// Generator index when requesting random numbers
  unsigned int _seed_index;

  /// Running index for the random number generators
  std::size_t _rand_index;

  /// Configuration values
  std::vector<std::vector<Real>> _confg_values;

  /// Vectors of new proposed samples combined with the experimental configuration values
  std::vector<std::vector<Real>> & _new_samples_confg;
};
