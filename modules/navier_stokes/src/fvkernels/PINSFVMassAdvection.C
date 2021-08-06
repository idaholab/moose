//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMassAdvection.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMassAdvection);

InputParameters
PINSFVMassAdvection::validParams()
{
  auto params = PINSFVMomentumAdvection::validParams();
  params.suppressParameter<MooseEnum>("momentum_component");
  params.addClassDescription("Object for advecting mass in porous media mass equation");
  return params;
}

PINSFVMassAdvection::PINSFVMassAdvection(const InputParameters & params)
  : PINSFVMomentumAdvection(params)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVMassAdvection::computeQpResidual()
{
  ADReal rho_interface;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  const auto v = _rc_uo.getVelocity(_velocity_interp_method, *_face_info, _tid);
  Moose::FV::interpolate(_advected_interp_method,
                         rho_interface,
                         _rho(elem_face),
                         _rho(neighbor_face),
                         v,
                         *_face_info,
                         true);
  return _normal * v * rho_interface;
}
