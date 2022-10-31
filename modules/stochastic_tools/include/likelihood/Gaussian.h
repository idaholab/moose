//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Likelihood.h"

/**
 * A class used to generate a Gaussian likelihood of observing model predictions
 */
class Gaussian : public Likelihood
{
public:
  static InputParameters validParams();

  Gaussian(const InputParameters & parameters);

  virtual Real densityFunction() const override;
  virtual Real massFunction() const override;

  static Real densityFunction(const std::vector<Real> & exp, const std::vector<Real> & model, const Real & noise, const bool & log_likelihood);

protected:
  /// return log-likelihood or likelihood
  const bool & _log_likelihood;

  /// model prediction values
  const std::vector<Real> & _model_pred;

  /// Noise value
  const Real & _noise;

  /// Experimental data values
  std::vector<Real> _exp_values;
  
};
