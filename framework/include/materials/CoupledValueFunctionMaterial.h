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

class Function;

/**
 * Compute a function value from coupled variables
 */
template <bool is_ad>
class CoupledValueFunctionMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  CoupledValueFunctionMaterialTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// output material property
  GenericMaterialProperty<Real, is_ad> & _prop;

  /// input function
  const Function & _function;

  /// mapping of variables to function parameters
  std::vector<unsigned int> _order;

  /// coupled variables to evaluate the function with
  std::vector<const GenericVariableValue<is_ad> *> _vals;

  /// number of coupled variables
  unsigned int _nvals;
};

typedef CoupledValueFunctionMaterialTempl<false> CoupledValueFunctionMaterial;
typedef CoupledValueFunctionMaterialTempl<true> ADCoupledValueFunctionMaterial;
