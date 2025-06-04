//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LikelihoodFunctionBase.h"
#include "ReporterInterface.h"

/**
 * A class used to generate a Gaussian likelihood of observing model predictions
 */
class Gaussian : public LikelihoodFunctionBase, public ReporterInterface
{
public:
  static InputParameters validParams();

  Gaussian(const InputParameters & parameters);

  virtual Real function(const std::vector<Real> & x) const override;

  /**
   * Return the probability density function
   * @param exp The experimental measurement
   * @param exp_group The experimental measurement group indicator
   * @param model The model prediction
   * @param noise The standard deviation
   * @param log_likelihood Bool to return the log likelihood value
   */
  static Real function(const std::vector<Real> & exp,
                       const std::vector<Real> & exp_group,
                       const std::vector<Real> & model,
                       const std::vector<Real> & noise,
                       const bool & log_likelihood);

protected:
  /// return log-likelihood or likelihood
  const bool _log_likelihood;

  /// Noise value
  const std::vector<Real> & _noise;

  /// Experimental data values
  std::vector<Real> _exp_values;

  /// Experimental configuration group indicators
  std::vector<Real> _exp_groups;
};
