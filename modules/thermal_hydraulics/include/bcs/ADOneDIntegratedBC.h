#pragma once

#include "ADIntegratedBC.h"

/**
 * Base class for integrated boundary conditions for 1D problems in 3D space
 */
class ADOneDIntegratedBC : public ADIntegratedBC
{
public:
  ADOneDIntegratedBC(const InputParameters & parameters);

protected:
  /// Component of outward normals along 1-D direction
  const Real _normal;

public:
  static InputParameters validParams();
};
