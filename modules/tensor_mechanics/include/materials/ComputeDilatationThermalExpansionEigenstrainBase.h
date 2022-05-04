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

/**
 * ComputeDilatationThermalExpansionEigenstrainBase computes an eigenstrain for thermal expansion
 * from an dilatation equation.
 */
class ComputeDilatationThermalExpansionEigenstrainBase
  : public ComputeThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeDilatationThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(Real & thermal_strain, Real * dthermal_strain_dT) override;

  /*
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual Real computeDilatation(const Real & temperature) = 0;

  /*
   * Compute the derivative of the fractional linear dilatation due to thermal expansion delta L / L
   * with respect to temperature
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual Real computeDilatationDerivative(const Real & temperature) = 0;
};
