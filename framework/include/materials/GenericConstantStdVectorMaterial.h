//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "VariableSizeMaterialPropertiesInterface.h"

/**
 * Material to create constant properties with the variable-size std::vector<Real> type
 */
template <bool is_ad>
class GenericConstantStdVectorMaterialTempl : public Material,
                                              public VariableSizeMaterialPropertiesInterface
{
public:
  static InputParameters validParams();

  GenericConstantStdVectorMaterialTempl(const InputParameters & parameters);

  virtual std::size_t getVectorPropertySize(const MaterialPropertyName & prop_name) const override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// The names of the constant vector material properties
  const std::vector<std::string> & _prop_names;

  /// The vector values of each vector material property
  const std::vector<std::vector<Real>> & _prop_values;

  /// The number of constant vector material properties defined
  std::size_t _num_props;

  /// A vector of pointers to the material properties
  std::vector<GenericMaterialProperty<std::vector<Real>, is_ad> *> _properties;
};

typedef GenericConstantStdVectorMaterialTempl<false> GenericConstantStdVectorMaterial;
typedef GenericConstantStdVectorMaterialTempl<true> ADGenericConstantStdVectorMaterial;
