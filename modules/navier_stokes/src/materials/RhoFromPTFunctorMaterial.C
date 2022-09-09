//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "RhoFromPTFunctorMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", RhoFromPTFunctorMaterial);

InputParameters
RhoFromPTFunctorMaterial::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addClassDescription(
      "Computes the density from coupled pressure and temperature functors (variables, "
      "functions, functor material properties");
  params.addRequiredParam<MooseFunctorName>(NS::temperature, "temperature functor");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "pressure functor");
  params.addParam<MooseFunctorName>(
      "density_name", NS::density, "name to use to declare the density functor");

  return params;
}

RhoFromPTFunctorMaterial::RhoFromPTFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _pressure(getFunctor<ADReal>(NS::pressure)),
    _temperature(getFunctor<ADReal>(NS::temperature)),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _density_name(getParam<MooseFunctorName>("density_name"))
{
  addFunctorProperty<ADReal>(_density_name,
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fluid.rho_from_p_T(_pressure(r, t), _temperature(r, t)); });
  addFunctorProperty<ADReal>(
      NS::time_deriv(_density_name),
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal rho, drho_dp, drho_dT;
        _fluid.rho_from_p_T(_pressure(r, t), _temperature(r, t), rho, drho_dp, drho_dT);
        return drho_dp * _pressure.dot(r, t) + drho_dT * _temperature.dot(r, t);
      });
}
