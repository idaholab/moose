#pragma once

#include "Material.h"

/**
 * Computes the product of a material property and a function.
 */
class ADMaterialFunctionProductMaterial : public Material
{
public:
  ADMaterialFunctionProductMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Product
  ADMaterialProperty<Real> & _product;
  /// Scale
  const ADMaterialProperty<Real> & _scale;
  /// Function
  const Function & _function;

public:
  static InputParameters validParams();
};
