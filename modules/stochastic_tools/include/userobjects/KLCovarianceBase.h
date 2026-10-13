//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Base class for the one-dimensional covariance marginals C(x1, x2) used by
 * KLExpansionUserObject. One instance is supplied per separable dimension of the expansion.
 */
class KLCovarianceBase : public GeneralUserObject
{
public:
  static InputParameters validParams();
  KLCovarianceBase(const InputParameters & parameters);

  /// Covariance between the one-dimensional coordinates x1 and x2
  virtual Real computeCovariance(Real x1, Real x2) const = 0;

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
};
