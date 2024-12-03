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
      "Computes the directed force coming from friction and pressure differences on a surface. One "
      "can use this object for the computation of the drag and lift coefficient as well.");
  params.addRequiredParam<MooseFunctorName>("vel_x", "The velocity in direction x.");
  params.addParam<MooseFunctorName>("vel_y", "The velocity in direction y.");
  params.addParam<MooseFunctorName>("vel_z", "The velocity in direction z.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure functor.");
  params.addRequiredParam<RealVectorValue>("principal_direction",
                                           "The direction in which the force is computed.");
  return params;
}

IntegralDirectedSurfaceForce::IntegralDirectedSurfaceForce(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _mu(getFunctor<Real>(NS::mu)),
    _pressure(getFunctor<Real>(NS::pressure)),
    _direction(getParam<RealVectorValue>("principal_direction"))
{
  _vel_components.push_back(&getFunctor<Real>("vel_x"));

  if (_mesh.dimension() >= 2)
  {
    if (!isParamValid("vel_y"))
      paramError("vel_y",
                 "For 2D meshes the second velocity component should be provided as well!");
    _vel_components.push_back(&getFunctor<Real>("vel_y"));

    if (_mesh.dimension() == 3)
    {
      if (!isParamValid("vel_z"))
        paramError("vel_z",
                   "For 3D meshes the third velocity component should be provided as well!");
      _vel_components.push_back(&getFunctor<Real>("vel_z"));
    }
  }

  // This object will only work with finite volume variables
  _qp_integration = false;

  checkFunctorSupportsSideIntegration<Real>("vel_x", _qp_integration);
  if (_vel_components.size() >= 2)
    checkFunctorSupportsSideIntegration<Real>("vel_y", _qp_integration);
  if (_vel_components.size() == 3)
    checkFunctorSupportsSideIntegration<Real>("vel_z", _qp_integration);
}

Real
IntegralDirectedSurfaceForce::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a face info in " + name());

  const auto state = determineState();
  const auto face_arg = Moose::FaceArg(
      {fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr});
  const auto elem_arg = Moose::ElemArg({fi->elemPtr(), false});

  RealTensorValue pressure_term;
  RealVectorValue cell_velocity = 0;
  RealVectorValue face_velocity = 0;
  const Real pressure = _pressure(face_arg, state);
  const Real mu = _mu(face_arg, state);
  for (const auto i : make_range(_mesh.dimension()))
  {
    cell_velocity(i) = (*_vel_components[i])(elem_arg, state);
    face_velocity(i) = (*_vel_components[i])(face_arg, state);
    pressure_term(i, i) = -pressure;
  }

  const auto shear_force = mu *
                           (cell_velocity - face_velocity -
                            (cell_velocity - face_velocity) * fi->normal() * fi->normal()) /
                           std::abs(fi->dCN() * fi->normal());

  return (shear_force * _direction - pressure_term * fi->normal() * _direction);
}

Real
IntegralDirectedSurfaceForce::computeQpIntegral()
{
  mooseError(this->type() + " does not have an implementation for quadrature-based evaluation!");
  return 0.0;
}
