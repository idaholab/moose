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
 * RankTwoInvariant uses the namespace RankTwoScalarTools to compute scalar
 * values from Rank-2 tensors.
 */
class RankTwoInvariant : public Material
{
public:
  static InputParameters validParams();

  RankTwoInvariant(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const MaterialProperty<RankTwoTensor> & _tensor;

  /// Name of the stress/strain to be calculated
  const std::string _property_name;

  /// Stress/strain value returned from calculation
  MaterialProperty<Real> & _property;

  /**
   * Determines the information to be extracted from the tensor by using the
   * RankTwoScalarTools namespace, e.g., max_principle , mid_principle, etc.
   */
  MooseEnum _invariant;

  /// The direction vector in which the scalar stress value is calculated
  Point _direction;
};
