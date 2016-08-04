/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EulerAngles.h"
#include "MooseRandom.h"

void
EulerAngles::random()
{
  phi1 = MooseRandom::rand() * 360.0;
  Phi  = std::acos(1.0 - 2.0 * MooseRandom::rand()) / libMesh::pi * 180.0;
  phi2 = MooseRandom::rand() * 360;
}
