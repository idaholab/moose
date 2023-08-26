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
 * Computes a vector material property from coupled variables
 */
template <bool is_ad>
class VectorFromComponentVariablesMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  VectorFromComponentVariablesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The velocity
  GenericMaterialProperty<RealVectorValue, is_ad> & _vector;
  /// The x-component
  const GenericVariableValue<is_ad> & _u;
  /// The y-component
  const GenericVariableValue<is_ad> & _v;
  /// The z-component
  const GenericVariableValue<is_ad> & _w;
};

typedef VectorFromComponentVariablesMaterialTempl<false> VectorFromComponentVariablesMaterial;
typedef VectorFromComponentVariablesMaterialTempl<true> ADVectorFromComponentVariablesMaterial;
