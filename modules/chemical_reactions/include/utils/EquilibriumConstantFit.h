//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "LeastSquaresFitBase.h"

/**
 * Fit the equilibrium constant values read from a databse at specified
 * temperature values with a Maier-Kelly type function for the equilibrium constant
 *
 * log(K)= a_0 ln(T) + a_1 + a_2 T + a_3 / T + a_4 / T^2
 *
 * where T is the temperature in Kelvin.
 *
 * Note: at least five data points must be provided to generate a fit
 */

class EquilibriumConstantFit : public LeastSquaresFitBase
{
public:
  EquilibriumConstantFit(const std::vector<Real> & temperature, const std::vector<Real> & logk);

  virtual Real sample(Real T) override;

protected:
  virtual void fillMatrix() override;
};
