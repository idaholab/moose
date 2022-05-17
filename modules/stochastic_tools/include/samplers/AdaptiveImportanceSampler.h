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

/**
 * A class used to perform Adaptive Importance Sampling using a Markov Chain Monte Carlo algorithm
 */
class AdaptiveImportanceSampler : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  AdaptiveImportanceSampler(const InputParameters & parameters);

  /// Access the initial values vector
  const std::vector<Real> & getInitialValues() const;

  /// Access the number of training samples
  const int & getNumSamplesTrain() const;

  /// Access use absolute value bool
  const bool & getUseAbsoluteValue() const;

  /// Access the output limit
  const Real & getOutputLimit() const;

  /// Access the mean vector of the importance distribution
  const std::vector<Real> & getImportanceVectorMean() const;

  /// Access the std vector of the importance distribution
  const std::vector<Real> & getImportanceVectorStd() const;

  /// Access the std vector of the importance distribution
  const std::vector<const Distribution *> & getDistributionNames() const;

  /// Access the output limit
  const Real & getStdFactor() const;

protected:
  /// Return the sample for the given row (the sample index) and column (the parameter index)
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<const Distribution *> _distributions;

  /// The proposal distribution standard deviations
  const std::vector<Real> & _proposal_std;

  /// Initial values values vector to start the importance sampler
  const std::vector<Real> & _initial_values;

  /// The output limit, exceedance of which indicates failure
  const Real & _output_limit;

  /// Number of samples to train the importance sampler
  const int & _num_samples_train;

  /// Factor to be multiplied to the standard deviation of the proposal distribution
  const Real & _std_factor;

  /// Absolute value of the model result. Use this when failure is defined as a non-exceedance rather than an exceedance.
  const bool & _use_absolute_value;

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int & _num_random_seeds;

private:
  /// Track the current step of the main App
  const int & _step;

  /// Storage for the inputs vector obtained from the reporter
  const std::vector<std::vector<Real>> & _inputs;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// For proposing the next sample in the MCMC algorithm
  std::vector<Real> _prev_value;

  /// Storage for means of input values for proposing the next sample
  std::vector<Real> _mean_sto;

  /// Storage for standard deviations of input values for proposing the next sample
  std::vector<Real> _std_sto;

  /// Storage for previously accepted samples by the decision reporter system
  std::vector<std::vector<Real>> _inputs_sto;
};
