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
   * Compute the probability density function at vector x (continuous case)
   */
  virtual Real densityFunction() const = 0;

  /**
   * Compute the probability mass function at vector x (discrete case)
   */
  virtual Real massFunction() const = 0;
};
