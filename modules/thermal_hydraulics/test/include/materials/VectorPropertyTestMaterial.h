#pragma once

#include "Material.h"

/**
 * Test material with vector properties
 */
class VectorPropertyTestMaterial : public Material
{
public:
  VectorPropertyTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<std::vector<Real>> & _vec;

public:
  static InputParameters validParams();
};
