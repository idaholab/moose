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

class MaterialRankTwoCylindricalScalar;

template <>
InputParameters validParams<MaterialRankTwoCylindricalScalar>();

/**
 * MaterialRankTwoCylindricalScalar uses the namespace RankTwoScalarTools to compute scalar
 * values from Rank-2 tensors.
 */
class MaterialRankTwoCylindricalScalar : public Material
{
public:
  static InputParameters validParams();

  MaterialRankTwoCylindricalScalar(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const MaterialProperty<RankTwoTensor> & _tensor;
  const std::string _calculation_name;

  MaterialProperty<Real> & _calculation;
  /**
   * Determines the information to be extracted from the tensor by using the
   * RankTwoScalarTools namespace, e.g., vonMisesStressL2norm, MaxPrincipal eigenvalue, etc.
   */
  MooseEnum _scalar_type;

  const Point _point1;
  const Point _point2;
  Point _input_direction;
};
