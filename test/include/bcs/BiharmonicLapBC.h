//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BIHARMONICLAPBC_H
#define BIHARMONICLAPBC_H

#include "IntegratedBC.h"

// Forward Declarations
class BiharmonicLapBC;

template <>
InputParameters validParams<BiharmonicLapBC>();

/**
 * The weak form of the biharmonic equation has a term
 * \int -Lap(u) * dv/dn ds
 * which we use to weakly impose the value of Lap(u) on the boundary.
 */
class BiharmonicLapBC : public IntegratedBC
{
public:
  BiharmonicLapBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// User-provided function which computes the Laplacian.
  Function & _lap_u;
};

#endif
