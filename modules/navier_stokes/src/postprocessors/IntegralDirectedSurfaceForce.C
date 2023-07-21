//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegralDirectedSurfaceForce.h"
#include "MathFVUtils.h"
#include "NSFVUtils.h"
#include "NS.h"
#include "MooseFunctorArguments.h"

#include <cmath>

registerMooseObject("NavierStokesApp", IntegralDirectedSurfaceForce);

InputParameters
IntegralDirectedSurfaceForce::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the directed force coming from friction and pressure differences on a surface. One can "
      "use this object for the computation of the drag and lift coefficient as well.");
  params.addRequiredParam<MooseFunctorName>("vel_x", "The velocity in direction x.");
  params.addParam<MooseFunctorName>("vel_y", "The velocity in direction y.");
  params.addParam<MooseFunctorName>("vel_z", "The velocity in direction z.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure functor.");
  params.addRequiredParam<RealVectorValue>("principal_direction",
                                           "The direction in which the drag is computed. This can "
                                           "be used to compute the lift coefficient as well.");
  return params;
}

IntegralDirectedSurfaceForce::IntegralDirectedSurfaceForce(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _vel_x(getFunctor<Real>("vel_x")),
    _vel_y(isParamValid("vel_y") ? &getFunctor<Real>("vel_y") : nullptr),
    _vel_z(isParamValid("vel_z") ? &getFunctor<Real>("vel_z") : nullptr),
    _mu(getFunctor<Real>(NS::mu)),
    _pressure(getFunctor<Real>(NS::pressure)),
    _direction(getParam<RealVectorValue>("principal_direction"))
{
  // This will only work with finite volume variables
  _qp_integration = false;
}

Real
IntegralDirectedSurfaceForce::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a face info in " + name());
  const auto state = determineState();
  const auto face_arg =
      Moose::FaceArg({fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr});

  const auto elem_arg = Moose::ElemArg({fi->elemPtr(), false});

  RealTensorValue pressure_term;
  Real pressure = _pressure(face_arg, state);
  for (auto i : make_range(_mesh.dimension()))
    pressure_term(i, i) = -pressure;


  Real mu = _mu(face_arg, state);
  RealVectorValue cell_velocity = 0;
  RealVectorValue face_velocity = 0;
  if (_mesh.dimension() == 1)
  {
    cell_velocity(0) = _vel_x(elem_arg, state);
    face_velocity(0) = _vel_x(face_arg, state);
  }
  else if (_mesh.dimension() == 2)
  {
    cell_velocity(0) = _vel_x(elem_arg, state);
    cell_velocity(1) = (*_vel_y)(elem_arg, state);
    face_velocity(0) = _vel_x(face_arg, state);
    face_velocity(1) = (*_vel_y)(face_arg, state);
  }
  else
  {
    cell_velocity(0) = _vel_x(elem_arg, state);
    cell_velocity(1) = (*_vel_y)(elem_arg, state);
    cell_velocity(2) = (*_vel_z)(elem_arg, state);
    face_velocity(0) = _vel_x(face_arg, state);
    face_velocity(1) = (*_vel_y)(face_arg, state);
    face_velocity(2) = (*_vel_z)(face_arg, state);
  }

  auto sheer_force = -mu *
                     (cell_velocity - face_velocity -
                      (cell_velocity - face_velocity) * fi->normal() * fi->normal()) /
                     std::abs(fi->dCN() * fi->normal());

  return (sheer_force * _direction + pressure_term * fi->normal() * _direction);
}

Real
IntegralDirectedSurfaceForce::computeQpIntegral()
{
  return 0.0;
}
