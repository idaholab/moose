//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCBase.h"

/**
 * A class for performing M-H MCMC sampling with independent Gaussian propoposals
 */
class IndependentGaussianMH : public PMCMCBase
{
public:
  static InputParameters validParams();

  IndependentGaussianMH(const InputParameters & parameters);

  virtual int decisionStep() const override { return 2; }

protected:
  virtual void proposeSamples(const unsigned int seed_value) override;

private:
  /// Reporter value the seed input values for proposing the next set of samples
  const std::vector<Real> & _seed_inputs;

  /// Standard deviations for making the next proposal
  const std::vector<Real> & _std_prop;
};
