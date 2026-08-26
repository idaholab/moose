//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "DiscreteSymmetry.h"

namespace Moose::MFEM
{

void
TranslationalSymmetry::ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out)
{
  mooseAssert((coord_in.Size() == _lattice_vector.Size()),
              "Size of lattice vector doesn't match the space dimension");
  add(coord_in, _lattice_vector, coord_out);
}

void
RotationalSymmetry::ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out)
{
  // x' =  x cos phi + y sin phi
  // y' = -x sin phi + y cos phi
  // z' = z
  coord_out[0] = coord_in[0] * cos(_rotation_angle) + coord_in[1] * sin(_rotation_angle);
  coord_out[1] = -coord_in[0] * sin(_rotation_angle) + coord_in[1] * cos(_rotation_angle);
  coord_out[2] = coord_in[2];
}

}

#endif
