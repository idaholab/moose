//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTFWallFunctionBC.h"
#include "Function.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVTFWallFunctionBC);

InputParameters
INSFVTFWallFunctionBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();

  // Description
  params.addClassDescription("Adds wall boundary condition for turbulent f (elliptic distribution "
                             "function or transfer from turbulent "
                             "kinetic energy to wall normal fluctuations) variable");

  // Coupled turbulent variables
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::TV2, "Coupled turbulent wall normal fluctuations.");

  // Coupled thermophysical properties
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");

  // Coupled closure parameters
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("n", 6.0, "Model parameter.");

  return params;
}

INSFVTFWallFunctionBC::INSFVTFWallFunctionBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _v2(getFunctor<ADReal>(NS::TV2)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _n(getParam<Real>("n"))
{
}

ADReal
INSFVTFWallFunctionBC::boundaryValue(const FaceInfo & fi) const
{
  // Convenient variables
  const Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto elem_arg = makeElemArg(&_current_elem);
  const auto state = determineState();
  const auto mu = _mu(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto nu = mu / rho;
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto TV2 = _v2(elem_arg, state);
  const auto C_mu = _C_mu(makeElemArg(&_current_elem), state);

  // Assign boundary weights to element
  // This is based on the theory of linear turbulence development for each boundary
  // This is, it assumes no interaction across turbulence production from boundaries
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());

  // Get friction velocity
  const ADReal u_star = std::pow(C_mu, 0.25) * std::sqrt(TKE);

  // Get associated non-dimensional wall distance
  const ADReal y_plus = u_star * dist / nu;

  if (y_plus <= 5.0) // sub-laminar layer
  {
    return 0.0;
  }
  else if (y_plus >= 30.0) // log-layer
  {
    return weight * _n * TV2 * TKED / (Utility::pow<2>(TKE) + 1e-15) /
           (Utility::pow<2>(u_star) + 1e-15);
  }
  else // blending function
  {
    const auto turbulent_value = weight * _n * TV2 * TKED / (Utility::pow<2>(TKE) + 1e-15) /
                                 (Utility::pow<2>(u_star) + 1e-15);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return interpolation_coef * turbulent_value;
  }
}
