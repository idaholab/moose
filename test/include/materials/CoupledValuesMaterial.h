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
 * A material that couples variables values and stores them into material property
 * This makes sure that everything is properly resized and can be indexed into.
 * @tparam whether to use the AD version, retrieving AD values from variables and storing them in
 *         AD material properties, or not
 */
template <bool is_ad>
class CoupledValuesMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  CoupledValuesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const GenericVariableValue<is_ad> & _value;
  const GenericVariableValue<is_ad> & _dot;
  const GenericVariableValue<is_ad> & _dot_dot;
  const VariableValue & _dot_du;
  const VariableValue & _dot_dot_du;

  const std::string & _var_name;
  GenericMaterialProperty<Real, is_ad> & _value_prop;
  GenericMaterialProperty<Real, is_ad> & _dot_prop;
  GenericMaterialProperty<Real, is_ad> & _dot_dot_prop;
  MaterialProperty<Real> & _dot_du_prop;
  MaterialProperty<Real> & _dot_dot_du_prop;
};

typedef CoupledValuesMaterialTempl<false> CoupledValuesMaterial;
typedef CoupledValuesMaterialTempl<true> ADCoupledValuesMaterial;
