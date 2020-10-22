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
 * Declares a constant material property of type RankTwoTensor.
 */
template <bool is_ad>
class GenericConstantRankTwoTensorTempl : public Material
{
public:
  static InputParameters validParams();

  GenericConstantRankTwoTensorTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  RankTwoTensor _tensor;
  GenericMaterialProperty<RankTwoTensor, is_ad> & _prop;
};

typedef GenericConstantRankTwoTensorTempl<false> GenericConstantRankTwoTensor;
typedef GenericConstantRankTwoTensorTempl<true> ADGenericConstantRankTwoTensor;
