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
  using ADComputeStressBaseTempl<R2>::_extra_stresses;  
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
  const Function & _functions00;
  const Function & _functions10;
  const Function & _functions20;
  const Function & _functions01;
  const Function & _functions11;
  const Function & _functions21;
  const Function & _functions02;
  const Function & _functions12;
  const Function & _functions22;

  const Function & _functions00es;
  const Function & _functions10es;
  const Function & _functions20es;
  const Function & _functions01es;
  const Function & _functions11es;
  const Function & _functions21es;
  const Function & _functions02es;
  const Function & _functions12es;
  const Function & _functions22es;


  SymmetricRankTwoTensor _initstress;
  SymmetricRankTwoTensor _initstrain;


};

typedef ADComputeStressBaseTempl<RankTwoTensor> ADComputeStressBase;
