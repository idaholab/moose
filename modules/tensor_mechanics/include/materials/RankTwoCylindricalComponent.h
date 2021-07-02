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

/// RankTwoCylindricalComponent computes cylindrical scalar values from Rank-2 tensors.
template <bool is_ad>
class RankTwoCylindricalComponentTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoCylindricalComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  RankTwoScalarTools::CylindricalComponent _cylindrical_component;

  /// Point 1 used to determine the axis of rotation
  const Point _cylindrical_axis_point1;
  /// Point 2 used to determine the axis of rotation
  const Point _cylindrical_axis_point2;
};

typedef RankTwoCylindricalComponentTempl<false> RankTwoCylindricalComponent;
typedef RankTwoCylindricalComponentTempl<true> ADRankTwoCylindricalComponent;
