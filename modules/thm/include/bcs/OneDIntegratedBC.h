#pragma once

#include "IntegratedBC.h"

class OneDIntegratedBC;

template <>
InputParameters validParams<OneDIntegratedBC>();

/**
 * Base class for integrated boundary conditions for 1D problems in 3D space
 */
class OneDIntegratedBC : public IntegratedBC
{
public:
  OneDIntegratedBC(const InputParameters & parameters);

protected:
  /// Component of outward normals along 1-D direction
  const Real _normal;
};
