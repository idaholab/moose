//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BodyForce.h"

/**
 * This kernel creates a body force that is defined by a material property. Common uses of this
 * would be to turn off or change the body force in certain regions of the mesh.
 */

class ADMatBodyForce : public ADBodyForce
{
public:
  static InputParameters validParams();

  ADMatBodyForce(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Material property used for body force strength
  const ADMaterialProperty<Real> & _property;
};
