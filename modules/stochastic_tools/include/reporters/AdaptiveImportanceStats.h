//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "AdaptiveImportanceSampler.h"

/**
 * AdaptiveImportanceStats will help make sample accept/reject decisions in adaptive Monte Carlo
 * schemes.
 */
class AdaptiveImportanceStats : public GeneralReporter
{
public:
  static InputParameters validParams();
  AdaptiveImportanceStats(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Model input data that is uncertain
  std::vector<Real> & _mu_imp;

  /// Model input data that is uncertain
  std::vector<Real> & _std_imp;

  /// Model input data that is uncertain
  std::vector<double> & _pf;

  /// Model input data that is uncertain
  std::vector<double> & _cov_pf;

private:
  /// Track the current step of the main App
  const int & _step;

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const AdaptiveImportanceSampler * const _ais;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  double _pf_sum;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  double _var_sum;

};
