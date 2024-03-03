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
#include "ADRankTwoTensorForward.h"

/**
 * ADStressDivergenceTensors is the automatic differentiation version of StressDivergenceTensors
 */
template <typename R2>
class ADStressDivergenceTensorsTempl : public ADKernel
{
public:
  static InputParameters validParams();

  ADStressDivergenceTensorsTempl(const InputParameters & parameters);

protected:
  void initialSetup() override;

  ADReal computeQpResidual() override;
  void precalculateResidual() override;

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// The stress tensor that the divergence operator operates on
  const ADMaterialProperty<R2> & _stress;

  /// An integer corresponding to the direction this kernel acts in
  const unsigned int _component;

  /// Number of coupled displacement variables
  const unsigned int _ndisp;

  /// Coupled displacement variable IDs
  std::vector<unsigned int> _disp_var;

  /// Gradient of test function averaged over the element. Used in volumetric locking correction calculation.
  std::vector<ADReal> _avg_grad_test;

  /// Whether out-of-plane strain is coupeld
  const bool _out_of_plane_strain_coupled;

  /// Pointer to the out-of-plane strain variable
  const ADVariableValue * const _out_of_plane_strain;

  /// Flag for volumetric locking correction
  const bool _volumetric_locking_correction;
};

typedef ADStressDivergenceTensorsTempl<RankTwoTensor> ADStressDivergenceTensors;
typedef ADStressDivergenceTensorsTempl<SymmetricRankTwoTensor> ADSymmetricStressDivergenceTensors;
