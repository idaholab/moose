//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVSymmetryVelocityBC.h"

registerMooseObject("NavierStokesApp", PINSFVSymmetryVelocityBC);

InputParameters
PINSFVSymmetryVelocityBC::validParams()
{
  return INSFVSymmetryVelocityBC::validParams();
}

PINSFVSymmetryVelocityBC::PINSFVSymmetryVelocityBC(const InputParameters & params)
  : INSFVSymmetryVelocityBC(params)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}
