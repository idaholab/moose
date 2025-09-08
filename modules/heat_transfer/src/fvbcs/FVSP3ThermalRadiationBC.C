//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSP3ThermalRadiationBC.h"
#include "Function.h"
#include "HeatTransferModels.h"

registerMooseObject("HeatTransferApp", FVSP3ThermalRadiationBC);

InputParameters
FVSP3ThermalRadiationBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Marshak Boundary Condition for SP3 Radiation Transport");

  params.addRequiredRangeCheckedParam<MooseFunctorName>(
      "Tb", "Tb>0", "The temperature of the boundary.");
  params.addRangeCheckedParam<Real>(
      "nu", "nu>0", "The mean frequency of the thermal radiation band.");
  params.addRangeCheckedParam<Real>(
      "nu_low", "nu_low>0", "The lower frequency for integration of the thermal radiation band.");
  params.addRangeCheckedParam<Real>(
      "nu_high",
      "nu_high>0",
      "The higher frequency for integration of the thermal radiation band.");

  params.addParam<MooseFunctorName>(
      "refraction_index", 1.0, "The refraction index in the spectral band.");

  params.addRequiredParam<MooseFunctorName>("epsilon", "The optical thickness of the medium.");
  params.addRequiredParam<MooseFunctorName>(
      "psi", "The incoming radiation heat flux moments from the conjugated SP3 order.");

  MooseEnum order("first second", "first");
  params.addParam<MooseEnum>("order", order, "The order of the diffusion term.");

  params.addRequiredParam<MooseFunctorName>("alpha", "First term Coefficient");
  params.addRequiredParam<MooseFunctorName>("beta", "Second term Coefficient");
  params.addRequiredParam<MooseFunctorName>("eta", "Third term Coefficient");

  params.addParam<std::string>("planck_units", "J*s", "Units for the Plank constant");
  params.addParam<std::string>("speedoflight_units", "m/s", "Units for the Speed of Light");
  params.addParam<std::string>("boltzmann_units", "J/K", "Units for the Boltzmann constant");

  return params;
}

FVSP3ThermalRadiationBC::FVSP3ThermalRadiationBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _Tb(getFunctor<ADReal>("Tb")),
    _nu(getParam<Real>("nu")),
    _nu_low(isParamValid("nu_low") ? &getParam<Real>("nu_low") : nullptr),
    _nu_high(isParamValid("nu_high") ? &getParam<Real>("nu_high") : nullptr),
    _n1(getFunctor<ADReal>("refraction_index")),
    _optical_thickness(getFunctor<ADReal>("epsilon")),
    _psi(getFunctor<ADReal>("psi")),
    _order(getParam<MooseEnum>("order")),
    _alpha(getFunctor<ADReal>("alpha")),
    _beta(getFunctor<ADReal>("beta")),
    _eta(getFunctor<ADReal>("eta")),
    _planck_units(getParam<std::string>("planck_units")),
    _sol_units(getParam<std::string>("speedoflight_units")),
    _boltzmann_units(getParam<std::string>("boltzmann_units"))
{
  if (_order == "first")
  {
    _squared_mu_order = _squared_mu_1;
  }
  else
  {
    _squared_mu_order = _squared_mu_2;
  }

  if (_nu_low && !_nu_high)
    paramError(
        "nu_high",
        "The top energy frequency for integration must be provided if the bottom one is provided.");

  if (!_nu_low && _nu_high)
    paramError(
        "nu_low",
        "The bottom energy frequency for integration must be provided if the top one is provided.");
}

ADReal
FVSP3ThermalRadiationBC::computeQpResidual()
{
  // Negative means incoming flux
  // Allow the functors to pick their side evaluation
  const Moose::FaceArg face{
      _face_info, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};
  const auto state = determineState();

  // Build the emission source at the boundary
  const auto Tb = _Tb(face, state);
  ADReal thermal_rad_source;

  if (_nu_low) // equation integrated by frequency band
  {
    const auto n1 = _n1(face, state);
    const auto nu_low = (*_nu_low);
    const auto nu_high = (*_nu_high);
    const Real abs_tol = 1E-8;
    const Real rel_tol = 1E-6;

    const auto rad_source = HeatTransferModels::integratedPlanckBand<ADReal>(n1,
                                                                             1.0,
                                                                             Tb,
                                                                             nu_low,
                                                                             nu_high,
                                                                             abs_tol,
                                                                             rel_tol,
                                                                             _planck_units,
                                                                             _sol_units,
                                                                             _boltzmann_units);
    thermal_rad_source = -1 * _eta(face, state) * rad_source / (4.0 * libMesh::pi);
  }
  else
  { // equation for a single frequency
    const auto n1_pow_2 = Utility::pow<2>(_n1(face, state));
    const auto nu = _nu;
    const auto nu_pow_3 = Utility::pow<3>(nu);
    const auto hp = HeatConduction::Constants::planckConstant(_planck_units);
    const auto c0 = HeatConduction::Constants::speedOfLight(_sol_units);
    const auto kb = HeatConduction::Constants::boltzmannConstant(_boltzmann_units);

    const auto pre_factor = n1_pow_2 * 2.0 * hp * nu_pow_3 / (Utility::pow<2>(c0));
    const auto inv_thermal_source = std::exp(hp * nu / (kb * Tb)) - 1.0;
    thermal_rad_source = -1 * _eta(face, state) * pre_factor / inv_thermal_source;
  }

  // Radiation leaking the boundary
  const auto thermal_rad_sink =
      _beta(face, state) * _psi(face, state) + _alpha(face, state) * _var(face, state);

  // Compute Marshak flux
  const auto protected_epsilon =
      (_optical_thickness(face, state) > 1e-12 ? _optical_thickness(face, state) : 1e-12);
  const auto flux = (thermal_rad_source + thermal_rad_sink) * protected_epsilon * _squared_mu_order;

  return flux;
}
