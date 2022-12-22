//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GaussianIID.h"
// #include "ReporterInterface.h"

/**
 * A class used to generate a truncated GaussianIID likelihood of observing model predictions
 * IID: Independent and Identically Distributed
 *
 * If more than one model and the corresponding weights are provided,
 * this capability generalizes to a truncated Gaussian mixture IID likelihood.
 */
class TruncatedGaussianIID : public GaussianIID
{
public:
  static InputParameters validParams();

  TruncatedGaussianIID(const InputParameters & parameters);

  virtual Real reqFunction(const std::vector<Real> & exp,
                           const std::vector<std::vector<Real>> & model,
                           const std::vector<Real> & weights,
                           const Real & noise,
                           const bool & log_likelihood) const override;

protected:
  /// Lower bounds
  const std::vector<Real> & _lb;

  /// Upper bounds
  const std::vector<Real> & _ub;
};
