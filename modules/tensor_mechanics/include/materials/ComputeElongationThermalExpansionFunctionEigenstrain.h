//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElongationThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeElongationThermalExpansionFunctionEigenstrain;

template <>
InputParameters validParams<ComputeElongationThermalExpansionFunctionEigenstrain>();

/**
 * ComputeElongationThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion from an elongation function.
 */
class ComputeElongationThermalExpansionFunctionEigenstrain
  : public ComputeElongationThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeElongationThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  /*
   * Compute the fractional linear elongation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual Real computeElongation(const Real & temperature) override;

  /*
   * Compute the derivative of the fractional linear elongation due to thermal expansion delta L / L
   * with respect to temperature
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual Real computeElongationDerivative(const Real & temperature) override;

  /// Elongation function
  const Function & _elongation_function;
};
