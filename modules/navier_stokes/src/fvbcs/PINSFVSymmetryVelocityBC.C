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
  InputParameters params = INSFVSymmetryVelocityBC::validParams();
  params.addClassDescription(
      "Implements a free slip boundary condition using a penalty formulation.");
  params.addRequiredCoupledVar("porosity", "The porosity.");
  return params;
}

PINSFVSymmetryVelocityBC::PINSFVSymmetryVelocityBC(const InputParameters & params)
  : INSFVSymmetryVelocityBC(params),
    _eps_var(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("porosity", 0)))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVSymmetryVelocityBC::computeQpResidual()
{
  const auto & eps_face = _eps_var->getBoundaryFaceValue(*_face_info);
  return INSFVSymmetryVelocityBC::computeQpResidual() / eps_face;
}
