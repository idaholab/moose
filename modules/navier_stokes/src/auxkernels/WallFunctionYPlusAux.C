//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallFunctionYPlusAux.h"
#include "NavierStokesMethods.h"
#include "CurvatureCorrec.h"

registerMooseObject("NavierStokesApp", WallFunctionYPlusAux);

InputParameters
WallFunctionYPlusAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates y+ value according to the algebraic velocity standard wall function.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho", "fluid density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls");
  return params;
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

WallFunctionYPlusAux::WallFunctionYPlusAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getFunctor<ADReal>("rho")),
    _mu(getFunctor<ADReal>("mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
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
  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
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

Real
WallFunctionYPlusAux::computeValue()
{
  const Elem & elem = *_current_elem;

  bool wall_bounded = false;
  Real min_wall_dist = 1e10;
  Point wall_vec;
  Point normal;
  for (unsigned int i_side = 0; i_side < elem.n_sides(); ++i_side)
  {
    const std::vector<BoundaryID> side_bnds =
        _subproblem.mesh().getBoundaryIDs(_current_elem, i_side);
    for (const BoundaryName & name : _wall_boundary_names)
    {
      BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
      for (BoundaryID side_id : side_bnds)
      {
        if (side_id == wall_id)
        {
          const FaceInfo * const fi = _mesh.faceInfo(&elem, i_side);
          const Point & this_normal = fi->normal();
          Point this_wall_vec = (elem.vertex_average() - fi->faceCentroid());
          Real dist = std::abs(this_wall_vec * normal);
          if (dist < min_wall_dist)
          {
            min_wall_dist = dist;
            wall_vec = this_wall_vec;
            normal = this_normal;
          }
          wall_bounded = true;
        }
      }
    }
  }

  if (!wall_bounded)
    return 0;

  const auto state = determineState();

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var->getElemValue(&elem, state));
  if (_v_var)
    velocity(1) = _v_var->getElemValue(&elem, state);
  if (_w_var)
    velocity(2) = _w_var->getElemValue(&elem, state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  Real dist = std::abs(wall_vec * normal);
  ADReal perpendicular_speed = velocity * normal;
  ADRealVectorValue parallel_velocity = velocity - perpendicular_speed * normal;
  ADReal parallel_speed = parallel_velocity.norm();
  ADRealVectorValue parallel_dir = parallel_velocity / parallel_speed;

  if (parallel_speed.value() < 1e-6)
    return 0;

  if (!std::isfinite(parallel_speed.value()))
    return parallel_speed.value();

  // Compute the friction velocity and the wall shear stress
  const auto elem_arg = makeElemArg(_current_elem);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  Real y_plus; 
  if (_curv_R)
  {
    ADReal speed_u = parallel_speed; // streamwise speed
    ADReal speed_w = 0;              // swirling speed
    // Getting curvature corrections
    if (_x_curvature_axis || _y_curvature_axis || _z_curvature_axis)
    {
      const auto x_curvature_axis = (*_x_curvature_axis)(elem_arg, state);
      const auto y_curvature_axis = (*_y_curvature_axis)(elem_arg, state);
      const auto z_curvature_axis = (*_z_curvature_axis)(elem_arg, state);
      if (x_curvature_axis > 1e-12 || y_curvature_axis > 1e-12 || y_curvature_axis > 1e-12)
      {
        ADRealVectorValue curv_axis(x_curvature_axis, y_curvature_axis, z_curvature_axis);
        ADRealVectorValue w_vector =
            normal.cross(curv_axis) / normal.cross(curv_axis).norm();
        speed_u =
            (velocity - velocity * normal * normal - velocity * w_vector * w_vector)
                  .norm();
        speed_w = (velocity * w_vector * w_vector).norm();
      }
    }

    // Get new stemwise friction velocity with swirling corrections
    const auto u_tau = NS::findUStar(mu, rho, speed_u, dist, _rough_ks);

    // Get swirling friction velocity
    ADReal w_tau = 0.0;
    const auto curv_R = (*_curv_R)(elem_arg, state);
    if (curv_R > 1e-12)
      w_tau = CurvatureCorrec::findWStar(
          mu.value(), rho.value(), speed_w, dist, curv_R, _convex);
    y_plus = (dist * std::sqrt(u_tau*u_tau + w_tau*w_tau) * rho / mu).value();
  }
  else
  {
    // Full Newton-Raphson solve to find the wall quantities from the law of the wall
    const auto u_tau = NS::findUStar(mu, rho, parallel_speed, dist, _rough_ks);
    y_plus = (dist * u_tau * rho / mu).value();
  }
  return y_plus;
}
