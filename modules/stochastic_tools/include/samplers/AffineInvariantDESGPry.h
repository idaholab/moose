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
#include "TransientInterface.h"
#include "Distribution.h"

/**
 * A base class used to perform Parallel Markov Chain Monte Carlo (MCMC) sampling
 */
class AffineInvariantDESGPry : public Sampler, public TransientInterface
{
public:
  static InputParameters validParams();

  AffineInvariantDESGPry(const InputParameters & parameters);

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
   * Return the proposed samples to facilitate decision making in reporters
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
  virtual int decisionStep() const { return 2; }

protected:
  /**
   * Fill in the _new_samples vector of vectors (happens within sampleSetUp)
   * @param seed_value The seed for the random number generator
   */
  virtual void proposeSamples(const unsigned int seed_value);

  // See Sampler.h for description
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;

  // See Sampler.h for description
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /**
   * Sample a random index excluding a specified index
   * @param upper_bound The upper bound provided
   * @param exclude The index to be excluded from sampling
   * @param seed The seed of the random number generator
   * @param req_index The required index to be filled
   */
  void randomIndex(const unsigned int & upper_bound,
                   const unsigned int & exclude,
                   const unsigned int & seed,
                   unsigned int & req_index);

  /**
   * Sample two random indices without repitition excluding a specified index
   * @param upper_bound The upper bound provided
   * @param exclude The index to be excluded from sampling
   * @param seed The seed of the random number generator
   * @param req_index1 The required index 1 to be filled
   * @param req_index2 The required index 2 to be filled
   */
  void randomIndexPair(const unsigned int & upper_bound,
                       const unsigned int & exclude,
                       const unsigned int & seed,
                       unsigned int & req_index1,
                       unsigned int & req_index2);

  /**
   * Compute the differential evolution from the current state
   * @param state1 A randomly selected state 1 of the sampler
   * @param state2 A randomly selected state 2 of the sampler
   * @param rnd Random number between 0 and 1
   * @param scale The scale of the input parameter
   * @param diff Differential between two randomly selected states
   */
  void computeDifferential(
      const Real & state1, const Real & state2, const Real & rnd, const Real & scale, Real & diff);

  /**
   * Tune the internal parameters
   * @param gamma An internal parameter
   * @param b An internal parameter
   * @param scale The scale of the input parameter
   */
  void tuneParams(Real & gamma, Real & b, const Real & scale);

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
  const Real * _variance_bound;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Initial values of the input params to get the MCMC scheme started
  const std::vector<Real> & _initial_values;

  /// Vectors of new proposed samples
  std::vector<std::vector<Real>> _new_samples;

  /// Vector of new proposed variance samples
  std::vector<Real> _new_var_samples;

  /// Vector of random numbers for decision making
  std::vector<Real> _rnd_vec;

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;

  /// Reporter value with the previous state of all the walkers for variance
  const std::vector<Real> & _previous_state_var;

  /// Tuning options for the internal params
  const MooseEnum & _tuning_option;

  /// Scales for the parameters
  std::vector<Real> _scales;

private:

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int _num_random_seeds;

  /// Starting input values
  std::vector<std::vector<Real>> _starting_values;
};
