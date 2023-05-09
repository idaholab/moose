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
}

ADReal
INSFVScalarFieldAdvection::computeQpResidual()
{
  const auto state = determineState();
  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, state, _tid);
  const auto var_face = _var(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0),
                             state);
  return _normal * v * var_face;
}
