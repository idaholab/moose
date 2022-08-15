//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeThermalExpansionEigenstrainBase.h"

/**
 * ComputeMeanThermalExpansionEigenstrainBase is a base class for computing the
 * thermal expansion eigenstrain according to a temperature-dependent mean thermal
 * expansion defined in a derived class.  This is defined as the total
 * total linear strain (\f$\delta L / L\f$) at a given temperature relative to a
 * reference temperature at which \f$\delta L = 0\f$.
 *
 * Based on:
 * M. Niffenegger and K. Reichlin. The proper use of thermal expansion coefficients
 * in finite element calculations. Nuclear Engineering and Design, 243:356-359, Feb. 2012.
 */
template <bool is_ad>
class ComputeMeanThermalExpansionEigenstrainBaseTempl
  : public ComputeThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeMeanThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters);

protected:
  /**
   * Compute the total thermal strain relative to the stress-free temperature at the
   * current temperature along with its temperature derivative.
   */
  virtual ValueAndDerivative<is_ad> computeThermalStrain() override;

  /**
   * Get the reference temperature for the mean thermal expansion relationship.  This is
   * the temperature at which \f$\delta L = 0\f$.
   */
  virtual Real referenceTemperature() = 0;

  /*
   * Compute the mean thermal expansion coefficient relative to the reference temperature
   * along with its temperature derivative.
   * This is the linear thermal strain divided by the temperature difference:
   * \f$\bar{\alpha}=(\delta L / L)/(T - T_{ref})\f$.
   * @param temperature  temperature at which this is evaluated
   */
  virtual ValueAndDerivative<is_ad>
  meanThermalExpansionCoefficient(const ValueAndDerivative<is_ad> & temperature) = 0;

  /// Scalar multiplier applied to the strain for sensitivity studies and debugging.
  const Real _thermal_expansion_scale_factor;

  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_qp;
};

typedef ComputeMeanThermalExpansionEigenstrainBaseTempl<false>
    ComputeMeanThermalExpansionEigenstrainBase;
typedef ComputeMeanThermalExpansionEigenstrainBaseTempl<true>
    ADComputeMeanThermalExpansionEigenstrainBase;
