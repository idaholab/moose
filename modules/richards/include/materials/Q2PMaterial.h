//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef Q2PMATERIAL_H
#define Q2PMATERIAL_H

#include "Material.h"

// Forward Declarations
class Q2PMaterial;

template <>
InputParameters validParams<Q2PMaterial>();

/**
 * Q2P Material.  Defines permeability, porosity and gravity
 */
class Q2PMaterial : public Material
{
public:
  Q2PMaterial(const InputParameters & parameters);

protected:
  /// porosity as entered by the user
  Real _material_por;

  /// porosity changes.  if not entered they default to zero
  const VariableValue & _por_change;
  const VariableValue & _por_change_old;

  /// permeability as entered by the user
  RealTensorValue _material_perm;

  /// gravity as entered by user
  RealVectorValue _material_gravity;

  /// material properties
  MaterialProperty<Real> & _porosity_old;
  MaterialProperty<Real> & _porosity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<RealVectorValue> & _gravity;

  std::vector<const VariableValue *> _perm_change;

  virtual void computeQpProperties();
};

#endif // Q2PMATERIAL_H
