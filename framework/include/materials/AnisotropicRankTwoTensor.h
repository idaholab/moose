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
#include "RankTwoTensor.h"

/**
 * Material object for defining an anisotropic rank two tensor property by specifying the three
 * principal components and optioonally the bases.
 */
template <bool is_ad>
class AnisotropicRankTwoTensorTempl : public Material
{
public:
  static InputParameters validParams();

  AnisotropicRankTwoTensorTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Rank two tensor property
  GenericMaterialProperty<RankTwoTensor, is_ad> & _prop;

  /// 1st principal component
  const GenericMaterialProperty<Real, is_ad> & _k1;
  /// 2nd principal component
  const GenericMaterialProperty<Real, is_ad> & _k2;
  /// 3rd principal component
  const GenericMaterialProperty<Real, is_ad> & _k3;

  /// 1st basis vector
  const RealVectorValue _e1;
  /// 2nd basis vector
  const RealVectorValue _e2;
  /// 3rd basis vector
  const RealVectorValue _e3;
};

typedef AnisotropicRankTwoTensorTempl<false> AnisotropicRankTwoTensor;
typedef AnisotropicRankTwoTensorTempl<true> ADAnisotropicRankTwoTensor;
