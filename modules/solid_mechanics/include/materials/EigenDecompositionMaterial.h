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
#include "RankTwoTensorForward.h"
/**
 * Perform eigendecomposition on a RankTwoTensor material property
 */
template <bool is_ad>
class EigenDecompositionMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  EigenDecompositionMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  /// Base name to allow multiple tensors to be decomposed
  const std::string _base_name;
  /// Rank two tensor for eigen decomposition
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// eigen vectors
  GenericMaterialProperty<RealVectorValue, is_ad> & _max_eigen_vector;
  GenericMaterialProperty<RealVectorValue, is_ad> & _mid_eigen_vector;
  GenericMaterialProperty<RealVectorValue, is_ad> & _min_eigen_vector;

  /// eigen values
  GenericMaterialProperty<Real, is_ad> & _max_eigen_value;
  GenericMaterialProperty<Real, is_ad> & _mid_eigen_value;
  GenericMaterialProperty<Real, is_ad> & _min_eigen_value;
};

typedef EigenDecompositionMaterialTempl<false> EigenDecompositionMaterial;
typedef EigenDecompositionMaterialTempl<true> ADEigenDecompositionMaterial;
