//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVTKEWallFunctionReichardtBC.h"
#include "Function.h"

registerMooseObject("MooseApp", NSFVTKEWallFunctionReichardtBC);

InputParameters
NSFVTKEWallFunctionReichardtBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription(
      "Adds Reichardt extrapolated wall values to set up directly the a Dirichlet BC for the turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addParam<bool>("linearized_yplus",
                        false,
                        "Boolean to indicate if yplus must be estimate locally for the blending functions.");
  params.addParam<Real>("min_mixing_length",
                        1.0,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for realizable k-epsilon answer.");
  return params;
}

NSFVTKEWallFunctionReichardtBC::NSFVTKEWallFunctionReichardtBC(const InputParameters & params)
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
    _C_mu(getFunctor<ADReal>("C_mu")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _min_mixing_length(getParam<Real>("min_mixing_length"))
{
}

Real
NSFVTKEWallFunctionReichardtBC::boundaryValue(const FaceInfo & fi) const
{
    Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
    const Elem & _current_elem = fi.elem();

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

    // Realizable constraint
    ADReal ralizable_constraint = std::pow(u_star / dist, 2);

    _console << "Realizable constraint: " << ralizable_constraint.value() << std::endl;
    _console << "Non Realizable value: " << (std::pow(u_star, 2) / std::sqrt(_C_mu(makeElemArg(&_current_elem)))).value() << std::endl;

    return std::max((std::pow(u_star, 2) / std::sqrt(_C_mu(makeElemArg(&_current_elem)))).value(),
                     ralizable_constraint.value());
}
