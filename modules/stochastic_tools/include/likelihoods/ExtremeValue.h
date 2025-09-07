//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Gaussian.h"

/**
 * A class used to generate a generalized extreme value likelihood of observing model predictions
 */
class ExtremeValue : public Gaussian
{
public:
  static InputParameters validParams();

  ExtremeValue(const InputParameters & parameters);

  virtual Real function(const std::vector<Real> & x) const override;

  /**
   * Return the probability density function
   * @param exp The experimental measurement
   * @param exp_group The experimental measurement group indicator
   * @param model The model prediction
   * @param noise The scale value
   * @param log_likelihood Bool to return the log likelihood value
   */
  static Real function(const std::vector<Real> & exp,
                       const std::vector<Real> & exp_group,
                       const std::vector<Real> & model,
                       const std::vector<Real> & noise,
                       const bool & log_likelihood);
};
