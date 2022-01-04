#pragma once

#include "Material.h"

class Function;

/**
 * Converts Darcy friction factor function into material property
 */
class ADWallFrictionFunctionMaterial : public Material
{
public:
  ADWallFrictionFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Function & _function;

  const MaterialPropertyName _f_D_name;
  ADMaterialProperty<Real> & _f_D;

public:
  static InputParameters validParams();
};
