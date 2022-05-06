//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeDilatationThermalExpansionEigenstrainBase.h"

/**
 * ComputeDilatationThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion from an dilatation function.
 */
template <bool is_ad>
class ComputeDilatationThermalExpansionFunctionEigenstrainTempl
  : public ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeDilatationThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters);

protected:
  /*
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual GenericReal<is_ad> computeDilatation(const GenericReal<is_ad> & temperature) override;

  /*
   * Compute the derivative of the fractional linear dilatation due to thermal expansion delta L / L
   * with respect to temperature (only called in the non-AD instantiation)
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual Real computeDilatationDerivative(const Real temperature) override;

  /// Dilatation function
  const Function & _dilatation_function;
};

typedef ComputeDilatationThermalExpansionFunctionEigenstrainTempl<false>
    ComputeDilatationThermalExpansionFunctionEigenstrain;
typedef ComputeDilatationThermalExpansionFunctionEigenstrainTempl<true>
    ADComputeDilatationThermalExpansionFunctionEigenstrain;
