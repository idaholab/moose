//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "EquilibriumConstantFit.h"
#include "PolynomialFit.h"

/**
 * Equilibrium constant (in the form log10(Keq)) calculated using a least-squares
 * fit to the data provided (typically taken from a geochemical database).
 *
 * Fitted function is a Maier-Kelly type function for the equilibrium constant
 *
 * log10(Keq)= a_0 ln(T) + a_1 + a_2 T + a_3 / T + a_4 / T^2
 *
 * where T is the temperature in Kelvin.
 *
 * Note: At least 5 data values must be provided in order to use the Maier-Kelley
 * fit. In some cases, insufficient data may be available. In these cases, the
 * following fits are used instead:
 *
 * If only one data point is available, log10(Keq) is set to a constant equal to the
 * given point.
 *
 * If between 2 and 4 data points are provided, a linear least squares fit is
 * used.
 */
class EquilibriumConstantAux : public AuxKernel
{
public:
  static InputParameters validParams();

  EquilibriumConstantAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Temperature (in K)
  const VariableValue & _temperature;
  /// Temperature points in data set (in K)
  const std::vector<Real> & _temperature_points;
  /// log(Keq) values at each temperature point
  const std::vector<Real> & _logk_points;
  /// Least-squares fit to data
  std::unique_ptr<EquilibriumConstantFit> _logk;
  /// Linear least-squares fit
  std::unique_ptr<PolynomialFit> _linear_logk;
};
