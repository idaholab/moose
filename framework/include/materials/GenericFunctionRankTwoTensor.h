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
 * Declares a function material property of type RankTwoTensor.
 */
template <bool is_ad>
class GenericFunctionRankTwoTensorTempl : public Material
{
public:
  static InputParameters validParams();

  GenericFunctionRankTwoTensorTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  GenericMaterialProperty<RankTwoTensor, is_ad> & _prop;
  const std::vector<FunctionName> _function_names;
  const unsigned int _num_functions;
  std::vector<const Function *> _functions;
};

typedef GenericFunctionRankTwoTensorTempl<false> GenericFunctionRankTwoTensor;
typedef GenericFunctionRankTwoTensorTempl<true> ADGenericFunctionRankTwoTensor;
