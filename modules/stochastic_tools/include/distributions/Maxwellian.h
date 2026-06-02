//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"
#include "NormalBase.h"
/**
 * A class used to generate a normal distribution
 */
class Maxwellian : public Distribution, public NormalBase
{
public:
  static InputParameters validParams();

  Maxwellian(const InputParameters & parameters);

  static Real pdf(const Real & x, const Real & mass, const Real & temperature);
  static Real cdf(const Real & x, const Real & mass, const Real & temperature);
  static Real quantile(const Real & p, const Real & mass, const Real & temperature);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

protected:
  /**
   * Helper for computing the standard deviation for the equivalent normal distribution
   * @param mass the pass of species at the given temperature in kg
   * @param temperature the temperature of the species in K
   * @returns the standard deviation of the equivalent normal distribution
   */
  static Real standardDeviation(const Real mass, const Real temperature);
};
