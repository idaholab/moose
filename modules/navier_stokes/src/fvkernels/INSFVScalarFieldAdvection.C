//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVScalarFieldAdvection.h"

registerMooseObject("NavierStokesApp", INSFVScalarFieldAdvection);

InputParameters
INSFVScalarFieldAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Advects an arbitrary quantity, the associated nonlinear 'variable'.");
  return params;
}

INSFVScalarFieldAdvection::INSFVScalarFieldAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVScalarFieldAdvection::computeQpResidual()
{
  ADReal var_interface;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  Moose::FV::interpolate(_advected_interp_method,
                         var_interface,
                         _var(elem_face),
                         _var(neighbor_face),
                         v,
                         *_face_info,
                         true);
  return _normal * v * var_interface;
}
