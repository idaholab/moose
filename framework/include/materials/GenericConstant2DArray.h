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

template <bool is_ad>
class GenericConstant2DArrayTempl : public Material
{
public:
  static InputParameters validParams();

  GenericConstant2DArrayTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  std::string _prop_name;
  const RealEigenMatrix & _prop_value;

  GenericMaterialProperty<RealEigenMatrix, is_ad> & _property;
};

typedef GenericConstant2DArrayTempl<false> GenericConstant2DArray;
typedef GenericConstant2DArrayTempl<true> ADGenericConstant2DArray;
