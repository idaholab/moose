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
