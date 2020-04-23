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

/// RankTwoCylindricalComponent computes cylindrical scalar values from Rank-2 tensors.
class RankTwoCylindricalComponent : public Material
{
public:
  static InputParameters validParams();

  RankTwoCylindricalComponent(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const MaterialProperty<RankTwoTensor> & _tensor;

  /// Name of the stress/strain to be calculated
  const std::string _property_name;

  /// Stress/strain value returned from calculation
  MaterialProperty<Real> & _property;

  MooseEnum _cylindrical_component;

  /// Point 1 used to determine the axis of rotation
  const Point _cylindrical_axis_point1;
  /// Point 2 used to determine the axis of rotation
  const Point _cylindrical_axis_point2;
};
