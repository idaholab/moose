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
#include "DerivativeMaterialInterface.h"

/**
 * ComputeDilatationThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion from an dilatation function.
 */
class ComputeDilatationThermalExpansionFunctionEigenstrain
  : public ComputeDilatationThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeDilatationThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  /*
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual Real computeDilatation(const Real & temperature) override;

  /*
   * Compute the derivative of the fractional linear dilatation due to thermal expansion delta L / L
   * with respect to temperature
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual Real computeDilatationDerivative(const Real & temperature) override;

  /// Dilatation function
  const Function & _dilatation_function;
};
