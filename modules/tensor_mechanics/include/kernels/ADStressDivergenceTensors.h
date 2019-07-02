//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

#define usingStressDivergenceTensorsMembers                                                        \
  usingKernelMembers;                                                                              \
  using ADStressDivergenceTensors<compute_stage>::_base_name;                                      \
  using ADStressDivergenceTensors<compute_stage>::_stress;                                         \
  using ADStressDivergenceTensors<compute_stage>::_component;                                      \
  using ADStressDivergenceTensors<compute_stage>::_ndisp;                                          \
  using ADStressDivergenceTensors<compute_stage>::_disp_var;                                       \
  using ADStressDivergenceTensors<compute_stage>::_avg_grad_test;                                  \
  using ADStressDivergenceTensors<compute_stage>::_volumetric_locking_correction

// Forward Declarations
template <ComputeStage>
class ADStressDivergenceTensors;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;

declareADValidParams(ADStressDivergenceTensors);

/**
 * ADStressDivergenceTensors is the automatic differentiation version of StressDivergenceTensors
 */
template <ComputeStage compute_stage>
class ADStressDivergenceTensors : public ADKernel<compute_stage>
{
public:
  ADStressDivergenceTensors(const InputParameters & parameters);

protected:
  void initialSetup() override;

  ADReal computeQpResidual() override;
  void precalculateResidual() override;

  const std::string _base_name;

  const ADMaterialProperty(RankTwoTensor) & _stress;
  const unsigned int _component;

  /// Coupled displacement variables
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;

  /// Gradient of test function averaged over the element. Used in volumetric locking correction calculation.
  std::vector<ADReal> _avg_grad_test;

  /// Flag for volumetric locking correction
  const bool _volumetric_locking_correction;

  usingKernelMembers;
};

