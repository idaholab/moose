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

/**
 * Declares a constant material property of type RealVectorValue.
 */
template <bool is_ad>
class GenericConstantRealVectorValueTempl : public Material
{
public:
  static InputParameters validParams();

  GenericConstantRealVectorValueTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const RealVectorValue _vector;
  GenericMaterialProperty<RealVectorValue, is_ad> & _prop;
};

typedef GenericConstantRealVectorValueTempl<false> GenericConstantRealVectorValue;
typedef GenericConstantRealVectorValueTempl<true> ADGenericConstantRealVectorValue;
