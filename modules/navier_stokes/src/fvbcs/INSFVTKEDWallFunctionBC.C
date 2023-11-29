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

registerMooseObject("NavierStokesApp", INSFVTKEDWallFunctionBC);

InputParameters
INSFVTKEDWallFunctionBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds Reichardt extrapolated wall values to set up directly the"
                             "Dirichlet BC for the turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("mu_t", "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  return params;
}

INSFVTKEDWallFunctionBC::INSFVTKEDWallFunctionBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _k(getFunctor<ADReal>("k")),
    _C_mu(getFunctor<ADReal>("C_mu"))
{
}

ADReal
INSFVTKEDWallFunctionBC::boundaryValue(const FaceInfo & fi) const
{
  Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto state = determineState();

  // Assign boundary weights to element
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
  {
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());
  }

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(makeElemArg(&_current_elem), state));
  if (_v_var)
    velocity(1) = (*_v_var)(makeElemArg(&_current_elem), state);
  if (_w_var)
    velocity(2) = (*_w_var)(makeElemArg(&_current_elem), state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  ADReal parallel_speed = (velocity - velocity * (fi.normal()) * (fi.normal())).norm();

  // Get friction velocity
  ADReal u_star = NS::findUStar(_mu(makeElemArg(&_current_elem), state),
                                _rho(makeElemArg(&_current_elem), state),
                                parallel_speed,
                                dist);

  // Get associated non-dimensional wall distance
  ADReal y_plus = dist * u_star * _rho(makeElemArg(&_current_elem), state) /
                  _mu(makeElemArg(&_current_elem), state);

  auto TKE = _k(makeElemArg(&_current_elem), state);

  if (y_plus <= 5.0) // sub-laminar layer
  {
    const auto laminar_value =
        2.0 * weight * TKE * _mu(makeElemArg(&_current_elem), state) / std::pow(dist, 2);
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
    const auto laminar_value =
        2.0 * weight * TKE * _mu(makeElemArg(&_current_elem), state) / std::pow(dist, 2);
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem), state) *
                                 std::pow(std::abs(TKE), 1.5) /
                                 (_mu_t(makeElemArg(&_current_elem), state) * dist);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return (interpolation_coef * (turbulent_value - laminar_value) + laminar_value).value();
  }
}
