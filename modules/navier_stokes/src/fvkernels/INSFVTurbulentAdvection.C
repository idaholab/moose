//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentAdvection.h"

registerMooseObject("NavierStokesApp", INSFVTurbulentAdvection);

InputParameters
INSFVTurbulentAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription(
      "Advects an arbitrary turbulent quantity, the associated nonlinear 'variable'.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  return params;
}

INSFVTurbulentAdvection::INSFVTurbulentAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params), _rho(getFunctor<ADReal>(NS::density))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVTurbulentAdvection::computeQpResidual()
{
  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  const auto var_face = _var(makeFace(
      *_face_info, limiterType(_advected_interp_method), MetaPhysicL::raw_value(v) * _normal > 0));
  const auto rho_face = _rho(makeFace(
      *_face_info, limiterType(_advected_interp_method), MetaPhysicL::raw_value(v) * _normal > 0));
  return _normal * v * rho_face * var_face;
}
