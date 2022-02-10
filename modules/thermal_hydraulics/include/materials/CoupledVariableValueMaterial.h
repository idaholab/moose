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
 * Stores values of a variable into material properties
 */
template <bool is_ad>
class CoupledVariableValueMaterialTempl : public Material
{
public:
  CoupledVariableValueMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The name of the material property where the values will be stored
  const MaterialPropertyName & _prop_name;
  /// Storage for the variable values
  GenericMaterialProperty<Real, is_ad> & _prop;
  /// The coupled variable values
  const VariableValue & _value;
  const ADVariableValue & _ad_value;

public:
  static InputParameters validParams();
};

typedef CoupledVariableValueMaterialTempl<false> CoupledVariableValueMaterial;
typedef CoupledVariableValueMaterialTempl<true> ADCoupledVariableValueMaterial;
