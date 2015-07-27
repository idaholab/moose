/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLES_H
#define EULERANGLES_H

#include "MooseTypes.h"
#include "libmesh/vector_value.h"

/**
 * Euler angle triplet.
 */
class EulerAngles
{
public:
  Real phi1, Phi, phi2;

  operator RealVectorValue() const { return RealVectorValue(phi1, Phi, phi2); }
};

#endif //EULERANGLES_H
