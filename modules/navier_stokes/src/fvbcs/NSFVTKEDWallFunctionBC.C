//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVTKEDWallFunctionBC.h"
#include "NS.h"
#include "NSEnums.h"

registerADMooseObject("NavierStokesApp", NSFVTKEDWallFunctionBC);

InputParameters
NSFVTKEDWallFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addParam<MooseFunctorName>(NS::porosity, 1.0, "Coupled porosity.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addClassDescription("Robin boundary condition for the turbulent kinetic energy dissipation.");
  return params;
}

NSFVTKEDWallFunctionBC::NSFVTKEDWallFunctionBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _k(getFunctor<ADReal>("k")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _C_mu(getFunctor<ADReal>("C_mu"))
{
}

ADReal
NSFVTKEDWallFunctionBC::computeQpResidual()
{
  // Get the element next to the boundar face
  const Elem & elem = _face_info->elem();
  auto current_argument = makeElemArg(&elem);

  // Compute multiplier for the Robin term
  constexpr Real karman_cte = 0.4187;
  auto friction_velocity = std::pow(_C_mu(current_argument), 0.25) * std::pow(_k(current_argument), 0.5);
  auto robin_multiplier = _eps(current_argument) * karman_cte * friction_velocity * _rho(current_argument) / _mu_t(current_argument);
  
  // Return RHS of Robin BC in residual form
  return _u[_qp] * robin_multiplier;
}