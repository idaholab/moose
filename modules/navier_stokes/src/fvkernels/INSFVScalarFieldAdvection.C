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
  params.addParam<MooseFunctorName>("u_slip", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The velocity in the z direction.");
  return params;
}

INSFVScalarFieldAdvection::INSFVScalarFieldAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_slip(isParamValid("u_slip") ? &getFunctor<ADReal>("u_slip") : nullptr),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr),
    _add_slip_model(isParamValid("u_slip") ? true : false)
{
  if (_add_slip_model)
  {
    if (_dim >= 2 && !_v_slip)
      mooseError(
          "In two or more dimensions, the v_slip velocity must be supplied using the 'v_slip' "
          "parameter");
    if (_dim >= 3 && !_w_slip)
      mooseError(
          "In three dimensions, the w_slip velocity must be supplied using the 'w_slip' parameter");
  }
}

ADReal
INSFVScalarFieldAdvection::computeQpResidual()
{
  const auto state = determineState();
  const auto & limiter_time = _subproblem.isTransient()
                                  ? Moose::StateArg(1, Moose::SolutionIterationType::Time)
                                  : Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);

  ADRealVectorValue advection_velocity;
  if (_add_slip_model)
  {

    Moose::FaceArg face_arg;
    if (onBoundary(*_face_info))
      face_arg = singleSidedFaceArg();
    else
      face_arg = Moose::FaceArg{_face_info,
                                Moose::FV::LimiterType::CentralDifference,
                                true,
                                false,
                                nullptr,
                                &limiter_time};

    ADRealVectorValue velocity_slip_vel_vec;
    if (_dim >= 1)
      velocity_slip_vel_vec(0) = (*_u_slip)(face_arg, state);
    if (_dim >= 2)
      velocity_slip_vel_vec(1) = (*_v_slip)(face_arg, state);
    if (_dim >= 3)
      velocity_slip_vel_vec(2) = (*_w_slip)(face_arg, state);
    advection_velocity += velocity_slip_vel_vec;
  }

  const auto v = velocity();
  advection_velocity += v;
  const auto var_face = _var(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0,
                                      false,
                                      &limiter_time),
                             state);

  return _normal * advection_velocity * var_face;
}
