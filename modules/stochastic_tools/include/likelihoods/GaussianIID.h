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
#include "ReporterInterface.h"

/**
 * A class used to generate a GaussianIID likelihood of observing model predictions
 * IID: Independent and Identically Distributed
 *
 * If more than one model and the corresponding weights are provided,
 * this capability generalizes to a Gaussian mixture IID likelihood.
 */
class GaussianIID : public Likelihood, public ReporterInterface
{
public:
  static InputParameters validParams();

  GaussianIID(const InputParameters & parameters);

  virtual Real function(const std::vector<std::vector<Real>> & x,
                        const std::vector<Real> & w) const override;

  virtual Real reqFunction(const std::vector<Real> & exp,
                           const std::vector<std::vector<Real>> & model,
                           const std::vector<Real> & weights,
                           const Real & noise,
                           const bool & log_likelihood) const;

protected:
  /// return log-likelihood or likelihood
  const bool & _log_likelihood;

  /// Noise value
  const Real & _noise;

  /// Experimental data values
  std::vector<Real> _exp_values;
};
