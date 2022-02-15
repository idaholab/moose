//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class FEProblemBase;

/**
 * Generate noise using random fourier series coefficients
 */
class FourierNoise : public Function
{
public:
  static InputParameters validParams();

  FourierNoise(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real, const Point & p) const override;
  virtual ADReal value(const ADReal &, const ADPoint & p) const override;

protected:
  struct SeriesItem
  {
    /// k-vector
    RealVectorValue k;
    /// sin coefficient
    Real s;
    /// cos coefficient
    Real c;
  };

  /// selected lower lengthscale for the noise cut-off
  const Real _lambda;

  /// Fourier series terms
  std::vector<SeriesItem> _series;

  /// amplitude factor
  Real _scale;

  /// FEProblem pointer for obtaining the current mesh
  FEProblemBase & _fe_problem;
};
