//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSP3ThermalRadiationSourceSink.h"
#include "MathUtils.h"
#include "HeatConductionNames.h"
#include "HeatTransferModels.h"

registerMooseObject("HeatTransferApp", FVSP3ThermalRadiationSourceSink);

InputParameters
FVSP3ThermalRadiationSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to the thermal radiation source and sink.");

  params.addRequiredRangeCheckedParam<MooseFunctorName>(
      "T", "T>0", "The temperature of the medium.");
  params.addRequiredRangeCheckedParam<Real>(
      "nu", "nu>0", "The mean frequency of the thermal radiation band.");
  params.addRangeCheckedParam<Real>(
      "nu_low", "nu_low>0", "The lower frequency for integration of the thermal radiation band.");
  params.addRangeCheckedParam<Real>(
      "nu_high",
      "nu_high>0",
      "The higher frequency for integration of the thermal radiation band.");
  params.addParam<MooseFunctorName>(
      "refraction_index", 1.0, "The refraction index in the spectral band.");
  params.addRequiredParam<MooseFunctorName>("kappa", "The absorptivity of the medium.");

  params.addParam<std::string>("planck_units", "J*s", "Units for the Plank constant");
  params.addParam<std::string>("speedoflight_units", "m/s", "Units for the Speed of Light");
  params.addParam<std::string>("boltzmann_units", "J/K", "Units for the Boltzmann constant");

  return params;
}

FVSP3ThermalRadiationSourceSink::FVSP3ThermalRadiationSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _T(getFunctor<ADReal>("T")),
    _nu(getParam<Real>("nu")),
    _nu_low(isParamValid("nu_low") ? &getParam<Real>("nu_low") : nullptr),
    _nu_high(isParamValid("nu_high") ? &getParam<Real>("nu_high") : nullptr),
    _n1(getFunctor<ADReal>("refraction_index")),
    _absorptivity(getFunctor<ADReal>("kappa")),
    _planck_units(getParam<std::string>("planck_units")),
    _sol_units(getParam<std::string>("speedoflight_units")),
    _boltzmann_units(getParam<std::string>("boltzmann_units"))

{
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
FVSP3ThermalRadiationSourceSink::computeQpResidual()
{
  // Convenient arguments
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem);
  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Time);

  // Build the emission source
  const auto T = _T(elem_arg, old_state);
  ADReal thermal_rad_source;

  if (_nu_low)
  {
    const auto n1 = _n1(elem_arg, state);
    const auto kappa = _absorptivity(elem_arg, state);
    const auto nu_low = (*_nu_low);
    const auto nu_high = (*_nu_high);
    const Real abs_tol = 1E-8;
    const Real rel_tol = 1E-6;

    thermal_rad_source = HeatTransferModels::integratedPlanckBand<ADReal>(n1,
                                                                          kappa,
                                                                          T,
                                                                          nu_low,
                                                                          nu_high,
                                                                          abs_tol,
                                                                          rel_tol,
                                                                          _planck_units,
                                                                          _sol_units,
                                                                          _boltzmann_units);
  }
  else
  {
    const auto n1_pow_2 = Utility::pow<2>(_n1(elem_arg, state));
    const auto nu = _nu;
    const auto nu_pow_3 = Utility::pow<3>(nu);
    const auto hp = HeatConduction::Constants::planckConstant(_planck_units);
    const auto c0 = HeatConduction::Constants::speedOfLight(_sol_units);
    const auto kb = HeatConduction::Constants::boltzmannConstant(_boltzmann_units);

    const auto pre_factor = n1_pow_2 * 2.0 * hp * nu_pow_3 / (Utility::pow<2>(c0));
    const auto inv_thermal_source = std::exp(hp * nu / (kb * T)) - 1.0;
    thermal_rad_source =
        4.0 * libMesh::pi * _absorptivity(elem_arg, state) * pre_factor / inv_thermal_source;
  }

  // Build the absorption sink
  const auto thermal_rad_sink = _absorptivity(elem_arg, state) * _var(elem_arg, state);

  // Return the residual
  return thermal_rad_sink - thermal_rad_source;
}
