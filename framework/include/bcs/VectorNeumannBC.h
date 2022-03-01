//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

#include "libmesh/vector_value.h"

/**
 * Implements a flux boundary condition grad(u).n = V.n, where the
 * vector V is specifed by the user. This differs from NeumannBC,
 * where the user instead specifies the _scalar_ value g = grad(u).n.
 */
class VectorNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  VectorNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Vector to dot with the normal.
  const RealVectorValue & _value;
};
