//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeDilatationThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ADComputeDilatationThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion from an dilatation function.
 */
class ADComputeDilatationThermalExpansionFunctionEigenstrain
  : public ADComputeDilatationThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ADComputeDilatationThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  /*
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual ADReal computeDilatation(const ADReal & temperature) override;

  /// Dilatation function
  const Function & _dilatation_function;
};
