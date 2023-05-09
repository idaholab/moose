//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVWallFunctionBC.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVWallFunctionBC);

InputParameters
INSFVWallFunctionBC::validParams()
{
  InputParameters params = INSFVNaturalFreeSlipBC::validParams();
  params.addClassDescription("Implements a wall shear BC for the momentum equation based on "
                             "algebraic standard velocity wall functions.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity");
  return params;
}

INSFVWallFunctionBC::INSFVWallFunctionBC(const InputParameters & params)
  : INSFVNaturalFreeSlipBC(params),
    _dim(_subproblem.mesh().dimension()),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu"))
{
}

ADReal
INSFVWallFunctionBC::computeStrongResidual()
{
  // Get the velocity vector
  const FaceInfo & fi = *_face_info;
  const Elem & elem = fi.elem();
  const Moose::ElemArg elem_arg{&elem, false};
  const auto state = determineState();
  ADRealVectorValue velocity(_u(elem_arg, state));
  if (_v)
    velocity(1) = (*_v)(elem_arg, state);
  if (_w)
    velocity(2) = (*_w)(elem_arg, state);

  // Compute the velocity magnitude (parallel_speed) and
  // direction of the tangential velocity component (parallel_dir)
  ADReal dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * _normal);
  ADReal perpendicular_speed = velocity * _normal;
  ADRealVectorValue parallel_velocity = velocity - perpendicular_speed * _normal;
  ADReal parallel_speed = parallel_velocity.norm();
  _a = 1 / parallel_speed;

  if (parallel_speed.value() < 1e-7)
    return 0;

  if (!std::isfinite(parallel_speed.value()))
    return parallel_speed;

  // Compute the friction velocity and the wall shear stress
  const auto rho = _rho(makeElemArg(&elem), state);
  ADReal u_star = NS::findUStar(_mu(makeElemArg(&elem), state), rho, parallel_speed, dist.value());
  ADReal tau = u_star * u_star * rho;
  _a *= tau;

  // Compute the shear stress component for this momentum equation
  if (_index == 0)
    return _a * parallel_velocity(0);
  else if (_index == 1)
    return _a * parallel_velocity(1);
  else
    return _a * parallel_velocity(2);
}

void
INSFVWallFunctionBC::gatherRCData(const FaceInfo & fi)
{
  _face_info = &fi;
  _face_type = fi.faceType(_var.name());
  _normal = fi.normal();

  // Fill-in the coefficient _a (but without multiplication by A)
  const auto strong_resid = computeStrongResidual();

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? &fi.elem() : fi.neighborPtr(),
                _index,
                _a * (fi.faceArea() * fi.faceCoord()));

  processResidualAndJacobian(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
