//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
