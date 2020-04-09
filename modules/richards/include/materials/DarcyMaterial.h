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

// Forward Declarations

/**
 * Defines the permeability tensor used in Darcy flow
 */
class DarcyMaterial : public Material
{
public:
  static InputParameters validParams();

  DarcyMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// permeability as entered by the user
  RealTensorValue _material_perm;

  /// the Material property that this Material provides
  MaterialProperty<RealTensorValue> & _permeability;
};
