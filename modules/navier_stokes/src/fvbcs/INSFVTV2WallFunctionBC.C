//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTV2WallFunctionBC.h"
#include "Function.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVTV2WallFunctionBC);

InputParameters
INSFVTV2WallFunctionBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();

  // Class description
  params.addClassDescription(
      "Adds wall boundary condition for turbulent v2 (wall normal stresses) variable");

  // Couple turbulent variables
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "The turbulent kinetic energy.");

  // Coupled thermophysical properties
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");

  // Closure parameters
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>(
      "Bv2", -0.94, "Near wall fitting constant for damping function logarithmic layer.");
  params.addParam<Real>("Cv2", 0.193, "Near wall fitting constant for damping function decay.");

  return params;
}

INSFVTV2WallFunctionBC::INSFVTV2WallFunctionBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getFunctor<ADReal>("C_mu")),
    Bv2_(getParam<Real>("Bv2")),
    Cv2_(getParam<Real>("Cv2"))
{
}

ADReal
INSFVTV2WallFunctionBC::boundaryValue(const FaceInfo & fi) const
{
  // Conveninet parameters
  const Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto state = determineState();
  const auto mu = _mu(makeElemArg(&_current_elem), state);
  const auto rho = _rho(makeElemArg(&_current_elem), state);
  const auto C_mu = _C_mu(makeElemArg(&_current_elem), state);
  const auto TKE = _k(makeElemArg(&_current_elem), state);

  // Assign boundary weights to element
  // This is based on the theory of linear TKE development for each boundary
  // This is, it assumes no interaction across turbulence production from boundaries
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());

  // Get friction velocity
  const ADReal u_star = std::pow(C_mu, 0.25) * std::sqrt(TKE);

  // Get associated non-dimensional wall distance
  const ADReal y_plus = dist * u_star * rho / mu;

  if (y_plus <= 5.0) // sub-laminar layer
  {
    return weight * Cv2_ * Utility::pow<4>(y_plus) * Utility::pow<2>(u_star);
  }
  else if (y_plus >= 30.0) // log-layer
  {
    return weight * (Cv2_ / NS::von_karman_constant * std::log(y_plus) + Bv2_) *
           Utility::pow<2>(u_star);
  }
  else // blending function
  {
    const auto laminar_value = Cv2_ * Utility::pow<4>(y_plus);
    const auto turbulent_value = (Cv2_ / NS::von_karman_constant * std::log(y_plus) + Bv2_);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return weight * (interpolation_coef * (turbulent_value - laminar_value) + laminar_value) *
           Utility::pow<2>(u_star);
  }
}
