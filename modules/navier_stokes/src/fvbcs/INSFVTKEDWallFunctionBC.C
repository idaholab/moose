//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKEDWallFunctionBC.h"
#include "Function.h"
#include "NavierStokesMethods.h"
#include "NS.h"
#include "CurvatureCorrec.h"

registerMooseObject("NavierStokesApp", INSFVTKEDWallFunctionBC);

InputParameters
INSFVTKEDWallFunctionBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds Reichardt extrapolated wall values to set up directly the"
                             "Dirichlet BC for the turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "The turbulent kinetic energy.");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
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

INSFVTKEDWallFunctionBC::INSFVTKEDWallFunctionBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getFunctor<ADReal>("C_mu")),
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
INSFVTKEDWallFunctionBC::boundaryValue(const FaceInfo & fi) const
{
  const Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto state = determineState();
  const auto mu = _mu(makeElemArg(&_current_elem), state);
  const auto rho = _rho(makeElemArg(&_current_elem), state);

  // Assign boundary weights to element
  // This is based on the theory of linear TKE development for each boundary
  // This is, it assumes no interaction across turbulence production from boundaries
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(makeElemArg(&_current_elem), state));
  if (_v_var)
    velocity(1) = (*_v_var)(makeElemArg(&_current_elem), state);
  if (_w_var)
    velocity(2) = (*_w_var)(makeElemArg(&_current_elem), state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  const ADReal parallel_speed = (velocity - velocity * (fi.normal()) * (fi.normal())).norm();

  // Get friction velocity
  ADReal y_plus = 0; 
  if (_curv_R)
  {
    ADReal speed_u = parallel_speed; // streamwise speed
    ADReal speed_w = 0;              // swirling speed
    // Getting curvature corrections
    if (_x_curvature_axis || _y_curvature_axis || _z_curvature_axis)
    {
      const auto x_curvature_axis = (*_x_curvature_axis)(makeElemArg(&_current_elem), state);
      const auto y_curvature_axis = (*_y_curvature_axis)(makeElemArg(&_current_elem), state);
      const auto z_curvature_axis = (*_z_curvature_axis)(makeElemArg(&_current_elem), state);
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
    const auto curv_R = (*_curv_R)(makeElemArg(&_current_elem), state);
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

  const auto TKE = _k(makeElemArg(&_current_elem), state);

  if (y_plus <= 5.0) // sub-laminar layer
  {
    const auto laminar_value = 2.0 * weight * TKE * mu / std::pow(dist, 2);
    return laminar_value.value();
  }
  else if (y_plus >= 30.0) // log-layer
  {
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem), state) *
                                 std::pow(std::abs(TKE), 1.5) /
                                 (_mu_t(makeElemArg(&_current_elem), state) * dist);
    return turbulent_value.value();
  }
  else // blending function
  {
    const auto laminar_value = 2.0 * weight * TKE * mu / std::pow(dist, 2);
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem), state) *
                                 std::pow(std::abs(TKE), 1.5) /
                                 (_mu_t(makeElemArg(&_current_elem), state) * dist);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return (interpolation_coef * (turbulent_value - laminar_value) + laminar_value).value();
  }
}
