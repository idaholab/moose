//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSP3TemperatureBC.h"
#include "Function.h"
#include "HeatTransferModels.h"

registerMooseObject("HeatTransferApp", FVSP3TemperatureBC);

InputParameters
FVSP3TemperatureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Semi-Transparent Boundary Condition for SP3 Radiation Temperature");

  params.addRequiredParam<MooseFunctorName>("Tb", "The temperature of the boundary");
  params.addParam<MooseFunctorName>(
      "n1", 1.0, "The refraction coefficient of incident medium (cavity).");
  params.addParam<MooseFunctorName>(
      "n2", 1.0, "The refraction coefficient of transmitting medium (boundary).");
  params.addRequiredParam<MooseFunctorName>(
      "h", "The convective heat transfer coefficient of incident medium.");
  params.addRequiredParam<MooseFunctorName>("k", "The thermal conductivity of incident medium.");
  params.addRequiredParam<MooseFunctorName>("epsilon", "The optical thickness of incident medium.");
  params.addRequiredParam<MooseFunctorName>("alpha",
                                            "The hemispheric emissivity of incident medium.");
  params.addRequiredParam<Real>("nu1", "The maximum opaque frequency.");
  params.addRequiredParam<Real>("nu_min", "The minimum frequency");

  params.addParam<std::string>("planck_units", "J*s", "Units for the Plank constant");
  params.addParam<std::string>("speedoflight_units", "m/s", "Units for the Speed of Light");
  params.addParam<std::string>("boltzmann_units", "J/K", "Units for the Boltzmann constant");

  return params;
}

FVSP3TemperatureBC::FVSP3TemperatureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _Tb(getFunctor<ADReal>("Tb")),
    _n1(getFunctor<ADReal>("n1")),
    _n2(getFunctor<ADReal>("n2")),
    _h(getFunctor<ADReal>("h")),
    _k(getFunctor<ADReal>("k")),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _alpha(getFunctor<ADReal>("alpha")),
    _nu1(getParam<Real>("nu1")),
    _nu_min(getParam<Real>("nu_min")),
    _planck_units(getParam<std::string>("planck_units")),
    _sol_units(getParam<std::string>("speedoflight_units")),
    _boltzmann_units(getParam<std::string>("boltzmann_units"))

{
}

ADReal
FVSP3TemperatureBC::computeQpResidual()
{
  // Allow the functors to pick their side evaluation
  const Moose::FaceArg face{
      _face_info, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};
  const auto state = determineState();

  // Build the convective source at the boundary
  const auto & T = _var(face, state);
  const auto & Tb = _Tb(face, state);
  const auto & epsilon = _epsilon(face, state);
  const auto & thermal_conv_source =
      _h(face, determineState()) * (_Tb(face, determineState()) - _var(face, determineState()));

  // Build the radiative source at the boundary
  const auto & n1 = _n1(face, state);
  const Real abs_tol = 1E-8;
  const Real rel_tol = 1E-6;

  const auto boundary_source = HeatTransferModels::integratedPlanckBand<ADReal>(
      n1, 1.0, Tb, _nu_min, _nu1, abs_tol, rel_tol, _planck_units, _sol_units, _boltzmann_units);
  const auto cell_source = HeatTransferModels::integratedPlanckBand<ADReal>(
      n1, 1.0, T, _nu_min, _nu1, abs_tol, rel_tol, _planck_units, _sol_units, _boltzmann_units);

  const auto thermal_rad_source =
      _alpha(face, state) * Utility::pow<2>(_n2(face, state) / _n1(face, state)) *
      (boundary_source - cell_source) / (4.0); // divided by 4 to get integrated PI*B(\nu)

  return -1 * (thermal_conv_source + thermal_rad_source) / epsilon;
}
