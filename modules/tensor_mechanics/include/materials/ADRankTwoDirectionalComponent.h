//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "RankTwoTensor.h"

/// ADRankTwoDirectionalComponent computes the component of a rank-2 tensor in specified direction
class ADRankTwoDirectionalComponent : public ADMaterial
{
public:
  static InputParameters validParams();

  ADRankTwoDirectionalComponent(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const ADMaterialProperty<RankTwoTensor> & _tensor;

  /// Name of the stress/strain to be calculated
  const std::string _property_name;

  /// Stress/strain value returned from calculation
  ADMaterialProperty<Real> & _property;

  /// The direction vector in which the scalar stress value is calculated
  const Point _direction;
};
