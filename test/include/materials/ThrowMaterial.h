//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THROW_MATERIAL_H
#define THROW_MATERIAL_H

#include "Material.h"

class ThrowMaterial;

template <>
InputParameters validParams<ThrowMaterial>();

/**
 * ThrowMaterial throws a MooseException when certain conditions are
 * met.
 */
class ThrowMaterial : public Material
{
public:
  ThrowMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Keeps _has_thrown synchronized between processors
  virtual void residualSetup() override;

  /// The MaterialProperty value we are responsible for computing
  MaterialProperty<Real> & _prop_value;

  /// The value of our MaterialProperty depends on the value of a coupled variable
  const VariableValue & _coupled_var;

  static bool _has_thrown;
};

#endif
