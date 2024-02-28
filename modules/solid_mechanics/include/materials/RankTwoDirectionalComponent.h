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

/// RankTwoDirectionalComponent computes the component of a rank-2 tensor in specified direction
template <bool is_ad>
class RankTwoDirectionalComponentTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoDirectionalComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  /// The direction vector in which the scalar stress value is calculated
  const Point _direction;
};

typedef RankTwoDirectionalComponentTempl<false> RankTwoDirectionalComponent;
typedef RankTwoDirectionalComponentTempl<true> ADRankTwoDirectionalComponent;
