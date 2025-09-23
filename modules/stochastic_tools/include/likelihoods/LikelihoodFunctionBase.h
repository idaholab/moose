//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"

/**
 * All Likelihoods should inherit from this class
 */
class LikelihoodFunctionBase : public MooseObject
{
public:
  static InputParameters validParams();
  LikelihoodFunctionBase(const InputParameters & parameters);

  /**
   * Return the probability density or mass function at vector x
   * @param x The input vector x
   */
  virtual Real function(const std::vector<Real> & x) const = 0;
};
