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
#include "SolidMaterialProperties.h"

/**
 * A class to define materials for the solid structures in the THM application.
 */
class ADSolidMaterial : public Material
{
public:
  ADSolidMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The solid material properties
  ADMaterialProperty<Real> & _thermal_conductivity;
  ADMaterialProperty<Real> & _specific_heat;
  ADMaterialProperty<Real> & _density;

  /// Temperature in the solid structure
  const ADVariableValue & _temp;
  /// User object with material properties
  const SolidMaterialProperties & _props;

public:
  static InputParameters validParams();
};
