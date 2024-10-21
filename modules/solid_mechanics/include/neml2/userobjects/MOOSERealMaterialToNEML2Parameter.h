//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2Parameter.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(MOOSERealMaterialToNEML2Parameter, MOOSEToNEML2Parameter);
#else

/**
 * Gather a MOOSE variable for insertion into the specified input of a NEML2 model.
 */
class MOOSERealMaterialToNEML2Parameter : public MOOSEToNEML2Parameter
{
public:
  static InputParameters validParams();

  MOOSERealMaterialToNEML2Parameter(const InputParameters & params);

protected:
  virtual torch::Tensor convertQpMOOSEData() const override;

  /// Coupled MOOSE variable to read data from
  const MaterialProperty<Real> & _mat_prop;
};

#endif
