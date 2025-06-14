//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralFunctorFluidProps.h"
#include "NS.h" // Variable Term Names
#include "HeatTransferUtils.h"
#include "NavierStokesMethods.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", GeneralFunctorFluidProps);
registerMooseObject("NavierStokesApp", NonADGeneralFunctorFluidProps);

template <bool is_ad>
InputParameters
GeneralFunctorFluidPropsTempl<is_ad>::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties functor userobject");
  params.addClassDescription("Creates functor fluid properties using a (P, T) formulation");

  params.addRequiredParam<MooseFunctorName>(NS::pressure, "Pressure");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "Fluid temperature");
  params.addRequiredParam<MooseFunctorName>(NS::speed, "Velocity norm");
  params.addParam<MooseFunctorName>(NS::density, "Density");
  params.addParam<bool>(
      "force_define_density",
      false,
      "Whether to force the definition of a density functor from the fluid properties");
  params.addParam<bool>("neglect_derivatives_of_density_time_derivative",
                        true,
                        "Whether to neglect the derivatives with regards to nonlinear variables "
                        "of the density time derivatives");

  params.addParam<FunctionName>(
      "mu_rampdown", 1, "A function describing a ramp down of viscosity over time");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "porosity");
  params.addRequiredParam<MooseFunctorName>(
      "characteristic_length", "characteristic length for Reynolds number calculation");

  // Dynamic pressure
  params.addParam<bool>("solving_for_dynamic_pressure",
                        false,
                        "Whether to solve for the dynamic pressure instead of the total pressure");
  params.addParam<Point>("reference_pressure_point",
                         Point(0, 0, 0),
                         "Point at which the gravity term for the static pressure is zero");
  params.addParam<Real>("reference_pressure", 1e5, "Total pressure at the reference point");
  params.addParam<Point>("gravity", Point(0, 0, -9.81), "Gravity vector");

  params.addParamNamesToGroup(
      "solving_for_dynamic_pressure reference_pressure_point reference_pressure",
      "Dynamic pressure");

  // Property names
  params.addParam<MooseFunctorName>(
      "density_name", NS::density, "Name to give to the density functor");
  params.addParam<MooseFunctorName>(
      "dynamic_viscosity_name", NS::mu, "Name to give to the dynamic viscosity functor");
  params.addParam<MooseFunctorName>(
      "specific_heat_name", NS::cp, "Name to give to the specific heat (cp) functor");
  params.addParam<MooseFunctorName>(
      "thermal_conductivity_name", NS::k, "Name to give to the thermal conductivity functor");
  params.addParamNamesToGroup(
      "density_name dynamic_viscosity_name specific_heat_name thermal_conductivity_name",
      "Functor property names");
  return params;
}

template <bool is_ad>
GeneralFunctorFluidPropsTempl<is_ad>::GeneralFunctorFluidPropsTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _eps(getFunctor<GenericReal<is_ad>>(NS::porosity)),
    _d(getFunctor<Real>("characteristic_length")),

    _pressure_is_dynamic(getParam<bool>("solving_for_dynamic_pressure")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _reference_pressure_value(getParam<Real>("reference_pressure")),
    _gravity_vec(getParam<Point>("gravity")),

    _pressure(getFunctor<GenericReal<is_ad>>(NS::pressure)),
    _T_fluid(getFunctor<GenericReal<is_ad>>(NS::T_fluid)),
    _speed(getFunctor<GenericReal<is_ad>>(NS::speed)),
    _force_define_density(getParam<bool>("force_define_density")),
    _rho(getFunctor<GenericReal<is_ad>>(NS::density)),
    _mu_rampdown(getFunction("mu_rampdown")),
    _neglect_derivatives_of_density_time_derivative(
        getParam<bool>("neglect_derivatives_of_density_time_derivative")),

    _density_name(getParam<MooseFunctorName>("density_name")),
    _dynamic_viscosity_name(getParam<MooseFunctorName>("dynamic_viscosity_name")),
    _specific_heat_name(getParam<MooseFunctorName>("specific_heat_name")),
    _thermal_conductivity_name(getParam<MooseFunctorName>("thermal_conductivity_name"))
{
  // Check parameters
  if (!_pressure_is_dynamic &&
      (isParamSetByUser("reference_pressure_point") || isParamSetByUser("reference_pressure")))
    paramError("solving_for_dynamic_pressure",
               "'reference_pressure_point' and 'reference_pressure' should not be set unless "
               "solving for the dynamic pressure");

  //
  // Set material properties functors
  //

  // If pressure is dynamic (else case), we must obtain the total pressure
  // We duplicate all this code. An alternative would be to redefine the pressure functor.
  // The issue with that is that you need the density functor to define the pressure,
  // and the pressure functor to define the density.
  // This could be solved by keeping a pointer to the pressure functor as an attribute and set
  // the pressure functor after the density functor has been defined.
  if (!_pressure_is_dynamic)
  {
    if (!isParamValid(NS::density) || _force_define_density)
      addFunctorProperty<GenericReal<is_ad>>(
          _density_name,
          [this](const auto & r, const auto & t) -> GenericReal<is_ad>
          { return _fluid.rho_from_p_T(_pressure(r, t), _T_fluid(r, t)); });

    addFunctorProperty<GenericReal<is_ad>>(
        NS::cv,
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return _fluid.cv_from_p_T(_pressure(r, t), _T_fluid(r, t)); });

    const auto & cp = addFunctorProperty<GenericReal<is_ad>>(
        _specific_heat_name,
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return _fluid.cp_from_p_T(_pressure(r, t), _T_fluid(r, t)); });

    const auto & mu = addFunctorProperty<GenericReal<is_ad>>(
        _dynamic_viscosity_name,
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return _mu_rampdown(r, t) * _fluid.mu_from_p_T(_pressure(r, t), _T_fluid(r, t)); });

    const auto & k = addFunctorProperty<GenericReal<is_ad>>(
        _thermal_conductivity_name,
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return _fluid.k_from_p_T(_pressure(r, t), _T_fluid(r, t)); });

    //
    // Time derivatives of fluid properties
    //
    if (_neglect_derivatives_of_density_time_derivative)
    {
      addFunctorProperty<GenericReal<is_ad>>(
          NS::time_deriv(_density_name),
          [this](const auto & r, const auto & t) -> GenericReal<is_ad>
          {
            Real rho, drho_dp, drho_dT;
            _fluid.rho_from_p_T(MetaPhysicL::raw_value(_pressure(r, t)),
                                MetaPhysicL::raw_value(_T_fluid(r, t)),
                                rho,
                                drho_dp,
                                drho_dT);
            return drho_dp * _pressure.dot(r, t) + drho_dT * _T_fluid.dot(r, t);
          });
    }
    else
    {
      addFunctorProperty<GenericReal<is_ad>>(
          NS::time_deriv(_density_name),
          [this](const auto & r, const auto & t) -> GenericReal<is_ad>
          {
            GenericReal<is_ad> rho, drho_dp, drho_dT;
            _fluid.rho_from_p_T(_pressure(r, t), _T_fluid(r, t), rho, drho_dp, drho_dT);
            return drho_dp * _pressure.dot(r, t) + drho_dT * _T_fluid.dot(r, t);
          });
    }

    addFunctorProperty<GenericReal<is_ad>>(
        NS::time_deriv(_specific_heat_name),
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          Real dcp_dp, dcp_dT, dummy;
          const auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          const auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);

          return dcp_dp * _pressure.dot(r, t) + dcp_dT * _T_fluid.dot(r, t);
        });

    //
    // Temperature and pressure derivatives, to help with computing time derivatives
    //

    const auto & drho_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_density_name, NS::pressure),
        [this](const auto & r, const auto & t) -> Real
        {
          Real drho_dp, drho_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
          return drho_dp;
        });

    const auto & drho_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_density_name, NS::T_fluid),
        [this](const auto & r, const auto & t) -> Real
        {
          Real drho_dp, drho_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
          return drho_dT;
        });

    const auto & dcp_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_specific_heat_name, NS::pressure),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dcp_dp, dcp_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
          return dcp_dp;
        });

    const auto & dcp_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_specific_heat_name, NS::T_fluid),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dcp_dp, dcp_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
          return dcp_dT;
        });

    const auto & dmu_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_dynamic_viscosity_name, NS::pressure),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dmu_dp, dmu_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
          return _mu_rampdown(r, t) * dmu_dp;
        });

    const auto & dmu_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_dynamic_viscosity_name, NS::T_fluid),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dmu_dp, dmu_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
          return _mu_rampdown(r, t) * dmu_dT;
        });

    const auto & dk_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_thermal_conductivity_name, NS::pressure),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dk_dp, dk_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
          return dk_dp;
        });

    const auto & dk_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_thermal_conductivity_name, NS::T_fluid),
        [this](const auto & r, const auto & t) -> Real
        {
          Real dk_dp, dk_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
          return dk_dT;
        });

    //
    // Fluid adimensional quantities, used in numerous correlations
    //

    addFunctorProperty<GenericReal<is_ad>>(
        NS::Prandtl,
        [&cp, &mu, &k](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;

          return HeatTransferUtils::prandtl(cp(r, t), mu(r, t), std::max(k(r, t), small_number));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Prandtl, NS::pressure),
        [&mu, &cp, &k, &dmu_dp, &dcp_dp, &dk_dp](const auto & r, const auto & t) -> Real
        {
          return NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(mu(r, t)),
                                               MetaPhysicL::raw_value(cp(r, t)),
                                               MetaPhysicL::raw_value(k(r, t)),
                                               dmu_dp(r, t),
                                               dcp_dp(r, t),
                                               dk_dp(r, t));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Prandtl, NS::T_fluid),
        [&mu, &cp, &k, &dmu_dT, &dcp_dT, &dk_dT](const auto & r, const auto & t) -> Real
        {
          return NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(mu(r, t)),
                                               MetaPhysicL::raw_value(cp(r, t)),
                                               MetaPhysicL::raw_value(k(r, t)),
                                               dmu_dT(r, t),
                                               dcp_dT(r, t),
                                               dk_dT(r, t));
        });

    //
    // (pore / particle) Reynolds number based on superficial velocity and
    // characteristic length. Only call Reynolds() one time to compute all three so that
    // we don't redundantly check that viscosity is not too close to zero.
    //

    const auto & Re = addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds,
        [this, &mu](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;
          return std::max(HeatTransferUtils::reynolds(_rho(r, t),
                                                      _eps(r, t) * _speed(r, t),
                                                      _d(r, t),
                                                      std::max(mu(r, t), small_number)),
                          small_number);
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Reynolds, NS::pressure),
        [this, &Re, &mu, &drho_dp, &dmu_dp](const auto & r, const auto & t) -> Real
        {
          return NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(Re(r, t)),
                                                MetaPhysicL::raw_value(_rho(r, t)),
                                                MetaPhysicL::raw_value(mu(r, t)),
                                                drho_dp(r, t),
                                                dmu_dp(r, t));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Reynolds, NS::T_fluid),
        [this, &Re, &mu, &drho_dT, &dmu_dT](const auto & r, const auto & t) -> Real
        {
          return NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(Re(r, t)),
                                                MetaPhysicL::raw_value(_rho(r, t)),
                                                MetaPhysicL::raw_value(mu(r, t)),
                                                drho_dT(r, t),
                                                dmu_dT(r, t));
        });

    // (hydraulic) Reynolds number
    addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds_hydraulic,
        [this, &Re](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;
          return Re(r, t) / std::max(1 - _eps(r, t), small_number);
        });

    // (interstitial) Reynolds number
    addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds_interstitial,
        [this, &Re](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return Re(r, t) / _eps(r, t); });
  }
  else
  {

    const auto & rho =
        (!isParamValid(NS::density) || _force_define_density)
            ? addFunctorProperty<GenericReal<is_ad>>(
                  _density_name,
                  [this](const auto & r, const auto & t) -> GenericReal<is_ad>
                  {
                    auto total_pressure = _pressure(r, t) + _reference_pressure_value;
                    // TODO: we should be integrating this term
                    const auto rho_approx = _fluid.rho_from_p_T(total_pressure, _T_fluid(r, t));
                    total_pressure +=
                        rho_approx * _gravity_vec * (r.getPoint() - _reference_pressure_point);
                    return _fluid.rho_from_p_T(total_pressure, _T_fluid(r, t));
                  })
            : getFunctor<GenericReal<is_ad>>(_density_name);

    addFunctorProperty<GenericReal<is_ad>>(
        NS::cv,
        [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          return _fluid.cv_from_p_T(total_pressure, _T_fluid(r, t));
        });

    const auto & cp = addFunctorProperty<GenericReal<is_ad>>(
        _specific_heat_name,
        [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          return _fluid.cp_from_p_T(total_pressure, _T_fluid(r, t));
        });

    const auto & mu = addFunctorProperty<GenericReal<is_ad>>(
        _dynamic_viscosity_name,
        [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          return _mu_rampdown(r, t) * _fluid.mu_from_p_T(total_pressure, _T_fluid(r, t));
        });

    const auto & k = addFunctorProperty<GenericReal<is_ad>>(
        _thermal_conductivity_name,
        [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          return _fluid.k_from_p_T(total_pressure, _T_fluid(r, t));
        });

    //
    // Time derivatives of fluid properties
    //
    if (_neglect_derivatives_of_density_time_derivative)
    {
      addFunctorProperty<GenericReal<is_ad>>(
          NS::time_deriv(_density_name),
          [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
          {
            const auto total_pressure =
                _pressure(r, t) + _reference_pressure_value +
                rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
            Real rho, drho_dp, drho_dT;
            _fluid.rho_from_p_T(MetaPhysicL::raw_value(total_pressure),
                                MetaPhysicL::raw_value(_T_fluid(r, t)),
                                rho,
                                drho_dp,
                                drho_dT);
            return drho_dp * _pressure.dot(r, t) + drho_dT * _T_fluid.dot(r, t);
          });
    }
    else
    {
      addFunctorProperty<GenericReal<is_ad>>(
          NS::time_deriv(_density_name),
          [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
          {
            const auto total_pressure =
                _pressure(r, t) + _reference_pressure_value +
                rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
            GenericReal<is_ad> rho, drho_dp, drho_dT;
            _fluid.rho_from_p_T(total_pressure, _T_fluid(r, t), rho, drho_dp, drho_dT);
            return drho_dp * _pressure.dot(r, t) + drho_dT * _T_fluid.dot(r, t);
          });
    }

    addFunctorProperty<GenericReal<is_ad>>(
        NS::time_deriv(_specific_heat_name),
        [this, &rho](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dcp_dp, dcp_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);

          return dcp_dp * _pressure.dot(r, t) + dcp_dT * _T_fluid.dot(r, t);
        });

    //
    // Temperature and pressure derivatives, to help with computing time derivatives
    //

    const auto & drho_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_density_name, NS::pressure),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real drho_dp, drho_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
          return drho_dp;
        });

    const auto & drho_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_density_name, NS::T_fluid),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real drho_dp, drho_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
          return drho_dT;
        });

    const auto & dcp_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_specific_heat_name, NS::pressure),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dcp_dp, dcp_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
          return dcp_dp;
        });

    const auto & dcp_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_specific_heat_name, NS::T_fluid),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dcp_dp, dcp_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
          return dcp_dT;
        });

    const auto & dmu_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_dynamic_viscosity_name, NS::pressure),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dmu_dp, dmu_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
          return _mu_rampdown(r, t) * dmu_dp;
        });

    const auto & dmu_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_dynamic_viscosity_name, NS::T_fluid),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dmu_dp, dmu_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
          return _mu_rampdown(r, t) * dmu_dT;
        });

    const auto & dk_dp = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_thermal_conductivity_name, NS::pressure),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dk_dp, dk_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
          return dk_dp;
        });

    const auto & dk_dT = addFunctorProperty<Real>(
        derivativePropertyNameFirst(_thermal_conductivity_name, NS::T_fluid),
        [this, &rho](const auto & r, const auto & t) -> Real
        {
          const auto total_pressure =
              _pressure(r, t) + _reference_pressure_value +
              rho(r, t) * _gravity_vec * (r.getPoint() - _reference_pressure_point);
          Real dk_dp, dk_dT, dummy;
          auto raw_pressure = MetaPhysicL::raw_value(total_pressure);
          auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));

          _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
          return dk_dT;
        });

    //
    // Fluid adimensional quantities, used in numerous correlations
    //

    addFunctorProperty<GenericReal<is_ad>>(
        NS::Prandtl,
        [&cp, &mu, &k](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;

          return HeatTransferUtils::prandtl(cp(r, t), mu(r, t), std::max(k(r, t), small_number));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Prandtl, NS::pressure),
        [&mu, &cp, &k, &dmu_dp, &dcp_dp, &dk_dp](const auto & r, const auto & t) -> Real
        {
          return NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(mu(r, t)),
                                               MetaPhysicL::raw_value(cp(r, t)),
                                               MetaPhysicL::raw_value(k(r, t)),
                                               dmu_dp(r, t),
                                               dcp_dp(r, t),
                                               dk_dp(r, t));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Prandtl, NS::T_fluid),
        [&mu, &cp, &k, &dmu_dT, &dcp_dT, &dk_dT](const auto & r, const auto & t) -> Real
        {
          return NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(mu(r, t)),
                                               MetaPhysicL::raw_value(cp(r, t)),
                                               MetaPhysicL::raw_value(k(r, t)),
                                               dmu_dT(r, t),
                                               dcp_dT(r, t),
                                               dk_dT(r, t));
        });

    //
    // (pore / particle) Reynolds number based on superficial velocity and
    // characteristic length. Only call Reynolds() one time to compute all three so that
    // we don't redundantly check that viscosity is not too close to zero.
    //

    const auto & Re = addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds,
        [this, &mu](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;
          return std::max(HeatTransferUtils::reynolds(_rho(r, t),
                                                      _eps(r, t) * _speed(r, t),
                                                      _d(r, t),
                                                      std::max(mu(r, t), small_number)),
                          small_number);
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Reynolds, NS::pressure),
        [this, &Re, &mu, &drho_dp, &dmu_dp](const auto & r, const auto & t) -> Real
        {
          return NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(Re(r, t)),
                                                MetaPhysicL::raw_value(_rho(r, t)),
                                                MetaPhysicL::raw_value(mu(r, t)),
                                                drho_dp(r, t),
                                                dmu_dp(r, t));
        });

    addFunctorProperty<Real>(
        derivativePropertyNameFirst(NS::Reynolds, NS::T_fluid),
        [this, &Re, &mu, &drho_dT, &dmu_dT](const auto & r, const auto & t) -> Real
        {
          return NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(Re(r, t)),
                                                MetaPhysicL::raw_value(_rho(r, t)),
                                                MetaPhysicL::raw_value(mu(r, t)),
                                                drho_dT(r, t),
                                                dmu_dT(r, t));
        });

    // (hydraulic) Reynolds number
    addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds_hydraulic,
        [this, &Re](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          static constexpr Real small_number = 1e-8;
          return Re(r, t) / std::max(1 - _eps(r, t), small_number);
        });

    // (interstitial) Reynolds number
    addFunctorProperty<GenericReal<is_ad>>(
        NS::Reynolds_interstitial,
        [this, &Re](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return Re(r, t) / _eps(r, t); });
  }
}

template class GeneralFunctorFluidPropsTempl<false>;
template class GeneralFunctorFluidPropsTempl<true>;
