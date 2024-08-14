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
#include "CurvatureCorrec.h"

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
  // Wall function correction parameters
  params.addParam<Real>("rough_ks", 0, "equivalent sand roughness height");
  params.addParam<MooseFunctorName>(NS::curv_R, 0, "curvature ");
  params.addParam<bool>("convex", false, "is the mesh convex ?");
  params.addParam<MooseFunctorName>(
      "x_curvature_axis", 0, "x coordinate of the axis along which the curvature is");
  params.addParam<MooseFunctorName>(
      "y_curvature_axis", 0, "y coordinate of the axis along which the curvature is");
  params.addParam<MooseFunctorName>(
      "z_curvature_axis", 0, "z coordinate of the axis along which the curvature is");
  params.addParamNamesToGroup(
      "rough_ks convex x_curvature_axis y_curvature_axis z_curvature_axis",
      "Wall function correction parameters");
  return params;
}

INSFVWallFunctionBC::INSFVWallFunctionBC(const InputParameters & params)
  : INSFVNaturalFreeSlipBC(params),
    _dim(_subproblem.mesh().dimension()),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _rough_ks(getParam<Real>("rough_ks")),
    _curv_R(params.isParamValid(NS::curv_R) ? &getFunctor<ADReal>(NS::curv_R) : nullptr),
    _convex(getParam<bool>("convex")),
    _x_curvature_axis(params.isParamValid("x_curvature_axis")
                          ? &getFunctor<ADReal>("x_curvature_axis")
                          : nullptr),
    _y_curvature_axis(params.isParamValid("y_curvature_axis")
                          ? &getFunctor<ADReal>("y_curvature_axis")
                          : nullptr),
    _z_curvature_axis(
        params.isParamValid("z_curvature_axis") ? &getFunctor<ADReal>("z_curvature_axis") : nullptr)
{
  if (_rc_uo.segregated())
    mooseError("Wall sheer stress enforcement based wall functions are not supported with "
               "segregated solution approaches!");
  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("x_curvature_axis"))
    paramError("x_curvature_axis",
               "Curvature in the `x` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("y_curvature_axis"))
    paramError("y_curvature_axis",
               "Curvature in the `y` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("z_curvature_axis"))
    paramError("z_curvature_axis",
               "Curvature in the `z` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (params.isParamValid(NS::curv_R) &&
      !(params.isParamValid("x_curvature_axis") && params.isParamValid("y_curvature_axis") &&
        params.isParamValid("z_curvature_axis")))
    mooseError("When curvature correction is active, all `x_curvature_axis`, `y_curvature_axis`, "
               "`z_curvature_axis` curvature axis need to be provided");
}

ADReal
INSFVWallFunctionBC::computeSegregatedContribution()
{
  mooseError("Sheer-stress-based wall function enforcement not supported for segregated solvers.");
  return 0.0;
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
  const auto mu = _mu(makeElemArg(&elem), state);
  if (_v)
    velocity(1) = (*_v)(elem_arg, state);
  if (_w)
    velocity(2) = (*_w)(elem_arg, state);

  // Compute the velocity magnitude (parallel_speed) and
  // direction of the tangential velocity component (parallel_dir)
  Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
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
  ADReal tau = 0;
  if (_curv_R)
  {
    ADReal speed_u = parallel_speed; // streamwise speed
    ADReal speed_w = 0;              // swirling speed
    // Getting curvature corrections
    if (_x_curvature_axis || _y_curvature_axis || _z_curvature_axis)
    {
      const auto x_curvature_axis = (*_x_curvature_axis)(makeElemArg(&elem), state);
      const auto y_curvature_axis = (*_y_curvature_axis)(makeElemArg(&elem), state);
      const auto z_curvature_axis = (*_z_curvature_axis)(makeElemArg(&elem), state);
      if (x_curvature_axis > 1e-12 || y_curvature_axis > 1e-12 || y_curvature_axis > 1e-12)
      {
        ADRealVectorValue curv_axis(x_curvature_axis, y_curvature_axis, z_curvature_axis);
        ADRealVectorValue w_vector =
            fi.normal().cross(curv_axis) / fi.normal().cross(curv_axis).norm();
        speed_u =
            (velocity - velocity * fi.normal() * fi.normal() - velocity * w_vector * w_vector)
                  .norm();
        speed_w = (velocity * w_vector * w_vector).norm();
      }
    }

    // Get new stemwise friction velocity with swirling corrections
    const auto u_tau = NS::findUStar(mu, rho, speed_u, dist, _rough_ks);

    // Get swirling friction velocity
    ADReal w_tau = 0.0;
    const auto curv_R = (*_curv_R)(makeElemArg(&elem), state);
    if (curv_R > 1e-12)
      w_tau = CurvatureCorrec::findWStar(
          mu, rho.value(), speed_w, dist, curv_R, _convex);
    tau = (u_tau * u_tau + w_tau * w_tau) * rho;
  }
  else
  {
    // Full Newton-Raphson solve to find the wall quantities from the law of the wall
    const auto u_tau = NS::findUStar(mu, rho, parallel_speed, dist, _rough_ks);
    tau = u_tau * u_tau * rho;
  }
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
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));
  _normal = fi.normal();

  // Fill-in the coefficient _a (but without multiplication by A)
  const auto strong_resid = computeStrongResidual();

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? &fi.elem() : fi.neighborPtr(),
                _index,
                _a * (fi.faceArea() * fi.faceCoord()));

  addResidualAndJacobian(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
