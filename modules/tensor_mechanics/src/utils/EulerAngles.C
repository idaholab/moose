//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EulerAngles.h"
#include "MooseRandom.h"

void
EulerAngles::random()
{
  phi1 = MooseRandom::rand() * 360.0;
  Phi = std::acos(1.0 - 2.0 * MooseRandom::rand()) / libMesh::pi * 180.0;
  phi2 = MooseRandom::rand() * 360;
}

void
EulerAngles::random(MooseRandom & random)
{
  phi1 = random.rand(0) * 360.0;
  Phi = std::acos(1.0 - 2.0 * random.rand(0)) / libMesh::pi * 180.0;
  phi2 = random.rand(0) * 360;
}
