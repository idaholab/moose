//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"

/**
 * Material class to compute the elastic free energy and its derivatives.
 */
template <bool is_ad>
class ElasticEnergyMaterialTempl : public DerivativeFunctionMaterialBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ElasticEnergyMaterialTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  usingDerivativeFunctionMaterialBaseMembers(is_ad);

  virtual GenericReal<is_ad> computeF() override;
  virtual GenericReal<is_ad> computeDF(unsigned int i_var) override;
  virtual GenericReal<is_ad> computeD2F(unsigned int i_var, unsigned int j_var) override;

  const std::string _base_name;

  /// Stress tensor
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;

  ///@{ Elasticity tensor and its explicit non-AD derivatives
  const GenericMaterialProperty<RankFourTensor, is_ad> & _elasticity_tensor;
  std::vector<const MaterialProperty<RankFourTensor> *> _delasticity_tensor;
  std::vector<std::vector<const MaterialProperty<RankFourTensor> *>> _d2elasticity_tensor;
  ///@}

  ///@{ Elastic strain and its explicit non-AD derivatives
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _strain;
  std::vector<const MaterialProperty<RankTwoTensor> *> _dstrain;
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *>> _d2strain;
  ///@}
};

typedef ElasticEnergyMaterialTempl<false> ElasticEnergyMaterial;
typedef ElasticEnergyMaterialTempl<true> ADElasticEnergyMaterial;
