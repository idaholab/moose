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

#define usingComputeElongationThermalExpansionEigenstrainBaseMembers                               \
  usingComputeThermalExpansionEigenstrainBaseMembers;                                              \
  using ADComputeElongationThermalExpansionEigenstrainBase<compute_stage>::computeElongation

template <ComputeStage compute_stage>
class ADComputeElongationThermalExpansionEigenstrainBase;

declareADValidParams(ADComputeElongationThermalExpansionEigenstrainBase);

/**
 * ADComputeElongationThermalExpansionEigenstrainBase computes an eigenstrain for thermal expansion
 * from an elongation equation.
 */
template <ComputeStage compute_stage>
class ADComputeElongationThermalExpansionEigenstrainBase
  : public ADComputeThermalExpansionEigenstrainBase<compute_stage>
{
public:
  static InputParameters validParams();

  ADComputeElongationThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(ADReal & thermal_strain) override;

  /*
   * Compute the fractional linear elongation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual ADReal computeElongation(const ADReal & temperature) = 0;

  usingComputeThermalExpansionEigenstrainBaseMembers;
};
