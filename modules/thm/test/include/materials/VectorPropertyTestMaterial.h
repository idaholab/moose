#pragma once

#include "Material.h"

class VectorPropertyTestMaterial;

template <>
InputParameters validParams<VectorPropertyTestMaterial>();

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
};
