//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EULERANGLES_H
#define EULERANGLES_H

#include "MooseTypes.h"
#include "libmesh/vector_value.h"

// forward declaration
class MooseRandom;

/**
 * Euler angle triplet.
 */
class EulerAngles
{
public:
  Real phi1, Phi, phi2;

  operator RealVectorValue() const { return RealVectorValue(phi1, Phi, phi2); }

  void random();
  void random(MooseRandom & random);
};

#endif // EULERANGLES_H
