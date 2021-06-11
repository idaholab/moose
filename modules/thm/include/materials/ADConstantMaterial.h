#pragma once

#include "Material.h"

/**
 * Constant material with zero-valued derivatives
 */
class ADConstantMaterial : public Material
{
public:
  ADConstantMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real & _value;

  const MaterialPropertyName _property_name;

  ADMaterialProperty<Real> & _property;

public:
  static InputParameters validParams();
};
