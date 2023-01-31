//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVTKEDWallFunctionReichardtBC.h"
#include "Function.h"
#include "NavierStokesMethods.h"

registerMooseObject("MooseApp", NSFVTKEDWallFunctionReichardtBC);

InputParameters
NSFVTKEDWallFunctionReichardtBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription(
      "Adds Reichardt extrapolated wall values to set up directly the a Dirichlet BC for the turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("mu_t", "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addParam<bool>("linearized_yplus",
                        false,
                        "Boolean to indicate if yplus must be estimate locally for the blending functions.");
  params.addParam<Real>("min_mixing_length",
                        1.0,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for realizable k-epsilon answer.");
  return params;
}

NSFVTKEDWallFunctionReichardtBC::NSFVTKEDWallFunctionReichardtBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v")
             ? &(getFunctor<ADReal>("v"))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? &(getFunctor<ADReal>("w"))
               : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _k(getFunctor<ADReal>("k")),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _min_mixing_length(getParam<Real>("min_mixing_length"))
{
}

Real
NSFVTKEDWallFunctionReichardtBC::boundaryValue(const FaceInfo & fi) const
{
  Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();

  // Assign boundary weights to element
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
  {
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());
  }
  //_console << "Weight: " << weight << std::endl;

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(makeElemArg(&_current_elem)));
  if (_v_var)
    velocity(1) = (*_v_var)(makeElemArg(&_current_elem));
  if (_w_var)
    velocity(2) = (*_w_var)(makeElemArg(&_current_elem));

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  ADReal parallel_speed  = (velocity - velocity * (fi.normal()) * (fi.normal())).norm();

  ADReal u_star;
  if (_linearized_yplus)
  {
    constexpr Real karman_cte = 0.4187;
    constexpr Real E = 9.793;
    const ADReal a_c = 1/karman_cte;
    const ADReal b_c = 1/karman_cte * (std::log(E*dist/_mu(makeElemArg(&_current_elem))) + 1.0);
    const ADReal c_c = parallel_speed;
    u_star = (-b_c + std::sqrt(std::pow(b_c,2)+4.0*a_c*c_c))/(2.0 * a_c);
  }
  else
    u_star = NS::findUStar(_mu(makeElemArg(&_current_elem)), _rho(makeElemArg(&_current_elem)), parallel_speed, dist);

  ADReal y_plus = dist * u_star * _rho(makeElemArg(&_current_elem)) / _mu(makeElemArg(&_current_elem));

  // _console << "Mu: " << _mu(makeElemArg(&_current_elem)) << std::endl;
  // _console << "Dist: " << dist << std::endl;
  // _console << "Weight: " << weight << std::endl;

  // auto TKE = std::max(_k(makeElemArg(&_current_elem)),
  //                    std::pow(_mu_t(makeElemArg(&_current_elem))
  //                             / _rho(makeElemArg(&_current_elem))
  //                             / _C_mu(makeElemArg(&_current_elem))
  //                             * std::abs(_var(makeElemArg(&_current_elem))), 0.5));

  auto TKE = _k(makeElemArg(&_current_elem));
  // _console << _current_elem.vertex_average() << std::endl;
  // _console << "TKE: " << TKE << std::endl;

  if (y_plus <= 5.0) //sub-laminar layer
  {
    const auto laminar_value = 2.0 * weight * TKE * _mu(makeElemArg(&_current_elem)) / std::pow(dist, 2);
    // _console << "Epsilon function: " << laminar_value << std::endl;
    return laminar_value.value();
  }
  else if(y_plus >= 30.0) // log-layer (actaully potential layer regarding modern research - change later)
  {
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem)) * std::pow(std::abs(TKE), 1.5) / (_mu_t(makeElemArg(&_current_elem)) * dist);
    return turbulent_value.value();
  }
  else // my experimental blending function
  {
    const auto laminar_value = 2.0 * weight * TKE * _mu(makeElemArg(&_current_elem)) / std::pow(dist, 2);
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem)) * std::pow(std::abs(TKE), 1.5) / (_mu_t(makeElemArg(&_current_elem)) * dist);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return (interpolation_coef * (turbulent_value - laminar_value) + laminar_value).value();
  }
}
