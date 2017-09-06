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

#ifndef VECTORNEUMANNBC_H
#define VECTORNEUMANNBC_H

#include "IntegratedBC.h"

#include "libmesh/vector_value.h"

// Forward Declarations
class VectorNeumannBC;

template <>
InputParameters validParams<VectorNeumannBC>();

/**
 * Implements a flux boundary condition grad(u).n = V.n, where the
 * vector V is specifed by the user. This differs from NeumannBC,
 * where the user instead specifies the _scalar_ value g = grad(u).n.
 */
class VectorNeumannBC : public IntegratedBC
{
public:
  VectorNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Vector to dot with the normal.
  const RealVectorValue & _value;
};

#endif
