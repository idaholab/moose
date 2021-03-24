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
class GenericConstantVectorMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  GenericConstantVectorMaterialTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// The names of the constant vector material properties
  std::vector<std::string> _prop_names;

  /// The vector values of each vector material property
  const std::vector<Real> & _prop_values;

  /// The number of constant vector material properties defined
  unsigned int _num_props;

  /// A vector of pointer to the material properties
  std::vector<GenericMaterialProperty<RealVectorValue, is_ad> *> _properties;
};

typedef GenericConstantVectorMaterialTempl<false> GenericConstantVectorMaterial;
typedef GenericConstantVectorMaterialTempl<true> ADGenericConstantVectorMaterial;
