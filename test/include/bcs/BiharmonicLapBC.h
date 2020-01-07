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

/**
 * The weak form of the biharmonic equation has a term
 * \int -Lap(u) * dv/dn ds
 * which we use to weakly impose the value of Lap(u) on the boundary.
 */
class BiharmonicLapBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  BiharmonicLapBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// User-provided function which computes the Laplacian.
  const Function & _lap_u;
};
