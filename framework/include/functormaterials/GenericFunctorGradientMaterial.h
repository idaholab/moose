//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * This material automatically declares as functor material properties whatever is passed to it
 * through the parameters 'prop_names' and uses the gradients of the functors from 'prop_values'
 * as the values for those properties.
 */
template <bool is_ad>
class GenericFunctorGradientMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  GenericFunctorGradientMaterialTempl(const InputParameters & parameters);

protected:
  /// Names of the functor material properties to define
  std::vector<std::string> _prop_names;

  /// Names of the functors to evaluate for those properties
  std::vector<MooseFunctorName> _prop_values;

  /// Number of properties to define
  unsigned int _num_props;

  /// Vector of the functors
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _functors;
};

typedef GenericFunctorGradientMaterialTempl<false> GenericFunctorGradientMaterial;
typedef GenericFunctorGradientMaterialTempl<true> ADGenericFunctorGradientMaterial;
