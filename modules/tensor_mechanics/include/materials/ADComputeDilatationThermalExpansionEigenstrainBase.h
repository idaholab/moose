//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

#define usingComputeDilatationThermalExpansionEigenstrainBaseMembers                               \
  usingComputeThermalExpansionEigenstrainBaseMembers;                                              \
  using ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>::computeDilatation

template <ComputeStage compute_stage>
class ADComputeDilatationThermalExpansionEigenstrainBase;

declareADValidParams(ADComputeDilatationThermalExpansionEigenstrainBase);

/**
 * ADComputeDilatationThermalExpansionEigenstrainBase computes an eigenstrain for thermal expansion
 * from an dilatation equation.
 */
template <ComputeStage compute_stage>
class ADComputeDilatationThermalExpansionEigenstrainBase
  : public ADComputeThermalExpansionEigenstrainBase<compute_stage>
{
public:
  static InputParameters validParams();

  ADComputeDilatationThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(ADReal & thermal_strain) override;

  /*
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear dilatation due
   */
  virtual ADReal computeDilatation(const ADReal & temperature) = 0;

  usingComputeThermalExpansionEigenstrainBaseMembers;
};
