//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeElongationThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

template <ComputeStage>
class ADComputeElongationThermalExpansionFunctionEigenstrain;

declareADValidParams(ADComputeElongationThermalExpansionFunctionEigenstrain);

/**
 * ADComputeElongationThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion from an elongation function.
 */
template <ComputeStage compute_stage>
class ADComputeElongationThermalExpansionFunctionEigenstrain
  : public ADComputeElongationThermalExpansionEigenstrainBase<compute_stage>
{
public:
  static InputParameters validParams();

  ADComputeElongationThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  /*
   * Compute the fractional linear elongation due to thermal expansion delta L / L
   * @param temperature current temperature
   * @return fractional linear elongation due
   */
  virtual ADReal computeElongation(const ADReal & temperature) override;

  /// Elongation function
  const Function & _elongation_function;

  usingComputeElongationThermalExpansionEigenstrainBaseMembers;
};
