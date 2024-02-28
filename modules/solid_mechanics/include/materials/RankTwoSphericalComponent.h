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
#include "RankTwoScalarTools.h"

/// RankTwoSphericalComponent computes spherical scalar values from Rank-2 tensors.
template <bool is_ad>
class RankTwoSphericalComponentTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoSphericalComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  RankTwoScalarTools::SphericalComponent _spherical_component;

  /// coordinate of the center
  const Point _center;
};

typedef RankTwoSphericalComponentTempl<false> RankTwoSphericalComponent;
typedef RankTwoSphericalComponentTempl<true> ADRankTwoSphericalComponent;
