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
#include "DerivativeMaterialInterface.h"

class ComputeElongationThermalExpansionEigenstrainBase;

template <>
InputParameters validParams<ComputeElongationThermalExpansionEigenstrainBase>();

/**
 * ComputeElongationThermalExpansionEigenstrainBase computes an eigenstrain for thermal expansion
 * from an elongation equation.
 */
class ComputeElongationThermalExpansionEigenstrainBase
  : public ComputeThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeElongationThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  /*
   * Compute the fractional linear elongation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual Real computeElongation(const Real & temperature) = 0;

  /*
   * Compute the derivative of the fractional linear elongation due to thermal expansion delta L / L
   * with respect to temperature
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual Real computeElongationDerivative(const Real & temperature) = 0;
};
