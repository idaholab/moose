/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DOUBLEWELLPOTENTIAL_H
#define DOUBLEWELLPOTENTIAL_H

#include "ACBulk.h"

// Forward Declarations
class DoubleWellPotential;

template <>
InputParameters validParams<DoubleWellPotential>();

/**
 * Algebraic double well potential.
 */
class DoubleWellPotential : public ACBulk<Real>
{
public:
  DoubleWellPotential(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
};

#endif // DOUBLEWELLPOTENTIAL_H
