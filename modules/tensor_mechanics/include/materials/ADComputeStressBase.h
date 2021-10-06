//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "Function.h"
#include "ADRankTwoTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"

#define usingComputeStressBaseMembers                                                              \
  usingMaterialMembers;                                                                            \
  using ADComputeStressBaseTempl<R2>::_base_name;                                                  \
  using ADComputeStressBaseTempl<R2>::_mechanical_strain;                                          \
  using ADComputeStressBaseTempl<R2>::_stress;                                                     \
  using ADComputeStressBaseTempl<R2>::_elastic_strain;                                             \
  using ADComputeStressBaseTempl<R2>::_extra_stresses;                                             \
  using ADComputeStressBaseTempl<R2>::_initial_stress_fcn

/**
 * ADComputeStressBaseTempl is the base class for stress tensors
 */
template <typename R2>
class ADComputeStressBaseTempl : public Material
{
public:
  static InputParameters validParams();

  ADComputeStressBaseTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void computeQpStress() = 0;

  /// Base name of the material system
  const std::string _base_name;

  const ADMaterialProperty<R2> & _mechanical_strain;

  /// The stress tensor to be calculated
  ADMaterialProperty<R2> & _stress;
  ADMaterialProperty<R2> & _elastic_strain;

  /// Extra stress tensors
  std::vector<const MaterialProperty<R2> *> _extra_stresses;

  /// initial stress components
  std::vector<const Function *> _initial_stress_fcn;
};

typedef ADComputeStressBaseTempl<RankTwoTensor> ADComputeStressBase;
