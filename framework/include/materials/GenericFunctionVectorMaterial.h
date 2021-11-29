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

template <bool>
class GenericFunctionVectorMaterialTempl;
typedef GenericFunctionVectorMaterialTempl<false> GenericFunctionVectorMaterial;
typedef GenericFunctionVectorMaterialTempl<true> ADGenericFunctionVectorMaterial;

/**
 * This material automatically declares as material properties whatever is passed to it
 * through the parameters 'prop_names' and uses the Functions from 'prop_values' as the values
 * for those properties.
 *
 * This is not meant to be used in a production capacity... and instead is meant to be used
 * during development phases for ultimate flexibility.
 */
template <bool is_ad>
class GenericFunctionVectorMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  GenericFunctionVectorMaterialTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Names of the vector material properties to define
  std::vector<std::string> _prop_names;

  /// The functions to use for each property's components
  std::vector<FunctionName> _prop_values;

  /// Number of vector properties that will be defined
  unsigned int _num_props;

  /// Vector of all the properties
  std::vector<GenericMaterialProperty<RealVectorValue, is_ad> *> _properties;

  /// Vector of pointers to the functions, stored here after retrieval using their name
  std::vector<const Function *> _functions;

private:
  /**
   * A helper method for evaluating the functions
   */
  void computeQpFunctions();
};
