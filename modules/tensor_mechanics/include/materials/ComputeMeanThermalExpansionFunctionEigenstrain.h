//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeMeanThermalExpansionEigenstrainBase.h"

/**
 * ComputeMeanThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to a mean thermal expansion function.
 */
template <bool is_ad>
class ComputeMeanThermalExpansionFunctionEigenstrainTempl
  : public ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeMeanThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters);

protected:
  /*
   * Get the reference temperature for the mean thermal expansion relationship.  This is
   * the temperature at which \f$\delta L = 0\f$.
   */
  virtual Real referenceTemperature() override;

  /*
   * Compute the mean thermal expansion coefficient relative to the reference temperature.
   * This is the linear thermal strain divided by the temperature difference:
   * \f$\bar{\alpha}=(\delta L / L)/(T - T_{ref})\f$.
   * param temperature  temperature at which this is evaluated
   */
  virtual ValueAndDerivative<is_ad>
  meanThermalExpansionCoefficient(const ValueAndDerivative<is_ad> & temperature) override;

  const Function & _thermal_expansion_function;

  const Real & _thexp_func_ref_temp;
};

typedef ComputeMeanThermalExpansionFunctionEigenstrainTempl<false>
    ComputeMeanThermalExpansionFunctionEigenstrain;
typedef ComputeMeanThermalExpansionFunctionEigenstrainTempl<true>
    ADComputeMeanThermalExpansionFunctionEigenstrain;
