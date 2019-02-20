#ifndef ONEDNODALBC_H
#define ONEDNODALBC_H

#include "NodalBC.h"

class OneDNodalBC;

template <>
InputParameters validParams<OneDNodalBC>();

/**
 * Base class for nodal boundary conditions for 1D problems in 3D space
 */
class OneDNodalBC : public NodalBC
{
public:
  OneDNodalBC(const InputParameters & parameters);

protected:
  /// Component of outward normal along 1-D direction
  const Real _normal;
};

#endif /* ONEDNODALBC_H */
