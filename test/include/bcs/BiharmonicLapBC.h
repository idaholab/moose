/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
