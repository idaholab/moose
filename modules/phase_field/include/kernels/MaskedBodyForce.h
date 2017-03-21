/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MASKEDBODYFORCE_H
#define MASKEDBODYFORCE_H

#include "BodyForce.h"

// Forward Declarations
class MaskedBodyForce;
class Function;

template <>
InputParameters validParams<MaskedBodyForce>();

/**
 * This kernel creates a body force that is modified by a mask defined
 * as a material. Common uses of this would be to turn off or change the
 * body force in certain regions of the mesh.
 */

class MaskedBodyForce : public BodyForce
{
public:
  MaskedBodyForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<Real> & _mask;
};

#endif
