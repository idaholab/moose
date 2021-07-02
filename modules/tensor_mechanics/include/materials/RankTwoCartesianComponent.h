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

/// ADRankTwoCartesianComponent computes selected components from a Rank-2 tensors.

template <bool is_ad>
class RankTwoCartesianComponentTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoCartesianComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  /// Tensor component
  const unsigned int _i;
  /// Tensor component
  const unsigned int _j;
};

typedef RankTwoCartesianComponentTempl<false> RankTwoCartesianComponent;
typedef RankTwoCartesianComponentTempl<true> ADRankTwoCartesianComponent;
