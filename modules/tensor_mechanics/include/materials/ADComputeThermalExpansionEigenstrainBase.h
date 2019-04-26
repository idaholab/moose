//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeEigenstrainBase.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#define usingComputeThermalExpansionEigenstrainBaseMembers                                         \
  usingComputeEigenstrainBaseMembers;                                                              \
  using ADComputeThermalExpansionEigenstrainBase<compute_stage>::_temperature;                     \
  using ADComputeThermalExpansionEigenstrainBase<compute_stage>::_deigenstrain_dT;                 \
  using ADComputeThermalExpansionEigenstrainBase<compute_stage>::_stress_free_temperature;         \
  using ADComputeThermalExpansionEigenstrainBase<compute_stage>::computeThermalStrain

template <ComputeStage>
class ADComputeThermalExpansionEigenstrainBase;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

declareADValidParams(ADComputeThermalExpansionEigenstrainBase);

/**
 * ADComputeThermalExpansionEigenstrainBase is a base class for all models that
 * compute eigenstrains due to thermal expansion of a material.
 */
template <ComputeStage compute_stage>
class ADComputeThermalExpansionEigenstrainBase : public ADComputeEigenstrainBase<compute_stage>,
                                                 public DerivativeMaterialPropertyNameInterface
{
public:
  ADComputeThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /*
   * Compute the total thermal strain relative to the stress-free temperature at
   * the current temperature, as well as the current instantaneous thermal
   * expansion coefficient.
   * param thermal_strain    The current total linear thermal strain
   *                         (\delta L / L)
   * param instantaneous_cte The current instantaneous coefficient of thermal
   *                         expansion (derivative of thermal_strain wrt
   *                         temperature
   */
  virtual void computeThermalStrain(ADReal & thermal_strain, ADReal & instantaneous_cte) = 0;

  const ADVariableValue & _temperature;
  ADMaterialProperty(RankTwoTensor) & _deigenstrain_dT;
  const ADVariableValue & _stress_free_temperature;

  usingComputeEigenstrainBaseMembers;
};

