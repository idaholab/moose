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
  ADRealVectorValue v;

  this->interpolate(_velocity_interp_method, v);
  const auto rho_interface =
      _rho(std::make_tuple(_face_info, _limiter.get(), v * _face_info->normal() > 0));
  return _normal * v * rho_interface;
}
