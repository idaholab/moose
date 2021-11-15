//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * through the parameters 'prop_names' and uses the Functions from 'prop_values' as the values
 * for those properties.
 */
template <bool is_ad>
class GenericFunctionVectorFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  GenericFunctionVectorFunctorMaterialTempl(const InputParameters & parameters);

protected:
  std::vector<std::string> _prop_names;
  std::vector<FunctionName> _prop_values;

  unsigned int _num_props;
  std::vector<const Function *> _functions;
};

typedef GenericFunctionVectorFunctorMaterialTempl<false> GenericFunctionVectorFunctorMaterial;
typedef GenericFunctionVectorFunctorMaterialTempl<true> ADGenericFunctionVectorFunctorMaterial;
