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
#include "DualRealOps.h"

/**
 * Fit the equilibrium constant values read from the thermodynamic databse at specified
 * temperature values with one of two types of fits:
 *
 * a fourth-order polynomial fit (for GWB databses)
 *
 * log(K)= a_0 + a_1 T + a_2 T^2 + a_3 T^3 + a_4 T^4
 *
 * a Maier-Kelly type fit (for EQ3/6 databases)
 *
 * log(K)= a_0 ln(T) + a_1 + a_2 T + a_3 / T + a_4 / T^2
 *
 * where T is the temperature in C.
 *
 * Note: at least five data points must be provided to generate a fit of either type.
 * If less than five data points exist (where 500.0000 represents no value), then a
 * linear fit is used.
 *
 * The type of fit is specified during construction using the type parameter.
 * The value of "no value" used in the database is provided using the optional no_value
 * parameter. The default value of 500 is used if this parameter is not specified.
 */

class EquilibriumConstantInterpolator : public LeastSquaresFitBase
{
public:
  EquilibriumConstantInterpolator(const std::vector<Real> & temperature,
                                  const std::vector<Real> & logk,
                                  const std::string type,
                                  const Real no_value = 500.0);

  virtual Real sample(Real T) override;
  DualReal sample(DualReal T);

  /**
   * Sample derivative of function at temperature T
   * @param T temperature
   * @return derivative of fit wrt T
   */
  Real sampleDerivative(Real T);

protected:
  virtual void fillMatrix() override;

  /// Functional form of fit
  enum class FitTypeEnum
  {
    FOURTHORDER,
    MAIERKELLY,
    LINEAR
  } _fit_type;
};
