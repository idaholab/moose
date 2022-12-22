//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "PerfGraphInterface.h"

/**
 * All Likelihoods should inherit from this class
 */
class Likelihood : public MooseObject, public PerfGraphInterface
{
public:
  static InputParameters validParams();

  Likelihood(const InputParameters & parameters);
  /**
   * Compute the probability density or mass function at x
   *
   * @param the model outputs vector of vector x
   * x.size() is the number of subApps (or models)
   * x[index].size() is the number of subApp (or model) evaluations
   *
   * @param the model weights vector w with w.size() = x.size()
   */
  virtual Real function(const std::vector<std::vector<Real>> & x,
                        const std::vector<Real> & w) const = 0;
};
