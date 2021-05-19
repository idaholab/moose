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

/// RankTwoInvariant computes invariant scalar values from Rank-2 tensors.
template <bool is_ad>
class RankTwoInvariantTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoInvariantTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Property to compute from the given tensor
  RankTwoScalarTools::InvariantType _invariant;

  /// Are stateful properties required?
  const bool _stateful;

  /// Tensor to extract the invariant from
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Old state of the tensor (required for effective strain calculation)
  const MaterialProperty<RankTwoTensor> * _tensor_old;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  /// The property needs to be stateful when computing effective strain
  const MaterialProperty<Real> * _property_old;
};

typedef RankTwoInvariantTempl<false> RankTwoInvariant;
typedef RankTwoInvariantTempl<true> ADRankTwoInvariant;
