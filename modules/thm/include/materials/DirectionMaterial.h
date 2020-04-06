#pragma once

#include "Material.h"

/**
 * Computes the directional vector of 1D elements in 3D space
 */
class DirectionMaterial : public Material
{
public:
  DirectionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The direction of the geometry (1D elements in 3D space)
  MaterialProperty<RealVectorValue> & _dir;

public:
  static InputParameters validParams();
};
