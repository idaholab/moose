//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
