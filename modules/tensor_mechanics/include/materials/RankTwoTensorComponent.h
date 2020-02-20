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
 * RankTwoTensorComponent is designed to take the data in the RankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */
template <bool is_ad>
class RankTwoTensorComponentTempl : public Material
{
public:
  static InputParameters validParams();

  RankTwoTensorComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;

  /// Name of the stress/strain to be calculated
  const std::string _property_name;

  /// Stress/strain value returned from calculation
  GenericMaterialProperty<Real, is_ad> & _property;

  /// Tensor components
  const unsigned int _i;
  const unsigned int _j;
};

typedef RankTwoTensorComponentTempl<false> RankTwoTensorComponent;
typedef RankTwoTensorComponentTempl<true> ADRankTwoTensorComponent;
