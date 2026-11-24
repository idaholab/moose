//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVScalarFieldAdvection.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", INSFVScalarFieldAdvection);

InputParameters
INSFVScalarFieldAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Advects an arbitrary quantity, the associated nonlinear 'variable'.");
  params.addParam<MooseFunctorName>(
      "u_slip",
      "The x component of a velocity the advected quantity carries relative to the mixture "
      "velocity, added to it to form the advecting velocity. For the two-phase mixture model "
      "this is the drift velocity of the dispersed phase, not its slip velocity.");
  params.addParam<MooseFunctorName>("v_slip", "The y component of that relative velocity.");
  params.addParam<MooseFunctorName>("w_slip", "The z component of that relative velocity.");
  params.deprecateParam("u_slip", "u_drift", "06/30/2027");
  params.deprecateParam("v_slip", "v_drift", "06/30/2027");
  params.deprecateParam("w_slip", "w_drift", "06/30/2027");
  return params;
}

INSFVScalarFieldAdvection::INSFVScalarFieldAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_slip(isParamValid("u_drift") ? &getFunctor<ADReal>("u_drift") : nullptr),
    _v_slip(isParamValid("v_drift") ? &getFunctor<ADReal>("v_drift") : nullptr),
    _w_slip(isParamValid("w_drift") ? &getFunctor<ADReal>("w_drift") : nullptr),
    _add_slip_model(isParamValid("u_drift") ? true : false)
{
  if (_add_slip_model)
    NS::checkVectorComponents(*this, _dim, _v_slip, _w_slip, "v_drift", "w_drift");
}

ADReal
INSFVScalarFieldAdvection::computeQpResidual()
{
  const auto state = determineState();
  const auto & limiter_time = _subproblem.isTransient()
                                  ? Moose::StateArg(1, Moose::SolutionIterationType::Time)
                                  : Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);

  const auto v = velocity();
  const auto var_face = _var(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0,
                                      false,
                                      &limiter_time),
                             state);
  auto flux = _normal * v * var_face;

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

    // The slip flux is interpolated on its own, with its own donor cell. Summing it with the
    // mixture flux and upwinding the sum on the mixture velocity alone hands a face carried by
    // the slip the wrong donor wherever the mixture velocity is small or opposed to the slip, a
    // wall cell for instance, and the advected quantity there drifts out of bounds.
    const auto var_face_slip =
        _var(makeFace(*_face_info,
                      limiterType(_advected_interp_method),
                      MetaPhysicL::raw_value(velocity_slip_vel_vec) * _normal > 0,
                      false,
                      &limiter_time),
             state);
    flux += _normal * velocity_slip_vel_vec * var_face_slip;
  }

  return flux;
}
