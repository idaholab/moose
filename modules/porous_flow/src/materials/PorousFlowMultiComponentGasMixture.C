//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMultiComponentGasMixture.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("PorousFlowApp", PorousFlowMultiComponentGasMixture);
registerMooseObject("PorousFlowApp", ADPorousFlowMultiComponentGasMixture);

template <bool is_ad>
InputParameters
PorousFlowMultiComponentGasMixtureTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMultiComponentFluidBaseTempl<is_ad>::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>("fp",
                                                       "FluidProperties for each gas component");
  params.addRequiredCoupledVar("mass_fraction",
                               "Mass fraction variables of gas components 0 to n-1 (The last "
                               "component mass fraction will be automatically computed) ");
  params.addClassDescription("This Material calculates the mixture fluid properties at the "
                             "quadpoints or nodes for a multiple component gas mixture");
  return params;
}

template <bool is_ad>
PorousFlowMultiComponentGasMixtureTempl<is_ad>::PorousFlowMultiComponentGasMixtureTempl(
    const InputParameters & parameters)
  : PorousFlowMultiComponentFluidBaseTempl<is_ad>(parameters),
    _fp_names(this->template getParam<std::vector<UserObjectName>>("fp")),
    _n_components(_fp_names.size()),
    _X_components(this->coupledComponents("mass_fraction"))
{
  if (_n_components < 2)
    this->template paramError("fp", "At least two gas components are required");

  if (_X_components != (_n_components - 1))
    this->template paramError(
        "mass_fraction",
        "The number of mass fraction variable must be equal to the number of fluid components-1");

  _fp.resize(_n_components);
  _M.resize(_n_components);

  for (unsigned int i = 0; i < _n_components; ++i)
  {
    _fp[i] = &this->template getUserObjectByName<SinglePhaseFluidProperties>(_fp_names[i]);
    _M[i] = _fp[i]->molarMass();
  }

  _X.resize(_X_components);
  for (unsigned int i = 0; i < _X_components; i++)
    _X[i] = (_nodal_material ? &this->template coupledGenericDofValue<is_ad>("mass_fraction", i)
                             : &this->template coupledGenericValue<is_ad>("mass_fraction", i));
}

template <bool is_ad>
void
PorousFlowMultiComponentGasMixtureTempl<is_ad>::initQpStatefulProperties()
{
  const auto p = MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) * _pressure_to_Pascals;
  const auto T = MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k;

  // Convert mass fractions (upper case) to mole fractions (lower case)
  std::vector<GenericReal<is_ad>> X(_n_components);
  for (const auto i : make_range(_X_components))
    X[i] = (*_X[i])[_qp];

  // The final mass fraction is 1 - sum_over_mass_fractions
  GenericReal<is_ad> total_mass_frac = 0.0;
  for (auto & Xi : X)
    total_mass_frac += Xi;

  X[_n_components - 1] = 1.0 - total_mass_frac;

  const auto x = massFractionsToMoleFractions(X);

  if (_compute_rho_mu)
  {
    GenericReal<is_ad> density_sum = 0.0;
    GenericReal<is_ad> viscosity_sum = 0.0;

    for (const auto i : make_range(_n_components))
    {
      density_sum += _fp[i]->rho_from_p_T(p, T) * x[i];
      viscosity_sum += _fp[i]->mu_from_p_T(p, T) * x[i];
    }

    (*_density)[_qp] = density_sum;
    (*_viscosity)[_qp] = viscosity_sum / _pressure_to_Pascals / _time_to_seconds;
  }

  if (_compute_internal_energy)
  {
    GenericReal<is_ad> e_sum = 0.0;

    for (const auto i : make_range(_n_components))
      e_sum += _fp[i]->e_from_p_T(p, T) * x[i];

    (*_internal_energy)[_qp] = e_sum;
  }

  if (_compute_enthalpy)
  {
    GenericReal<is_ad> h_sum = 0.0;

    for (const auto i : make_range(_n_components))
      h_sum += _fp[i]->h_from_p_T(p, T) * x[i];

    (*_enthalpy)[_qp] = h_sum;
  }
}

template <bool is_ad>
void
PorousFlowMultiComponentGasMixtureTempl<is_ad>::computeQpProperties()
{
  const auto p = _porepressure[_qp][_phase_num] * _pressure_to_Pascals;
  const auto T = _temperature[_qp] + _t_c2k;

  // Convert mass fractions (upper case) to mole fractions (lower case)
  std::vector<GenericReal<is_ad>> X(_n_components);
  for (const auto i : make_range(_X_components))
    X[i] = (*_X[i])[_qp];

  // The final mass fraction is 1 - sum_over_mass_fractions
  GenericReal<is_ad> total_mass_frac = 0.0;
  for (auto & Xi : X)
    total_mass_frac += Xi;

  X[_n_components - 1] = 1.0 - total_mass_frac;

  const auto x = massFractionsToMoleFractions(X);

  // Derivative of mole fractions wrt mass fractions
  const auto dx = dMoleFraction(X);

  if (_compute_rho_mu)
  {
    if (is_ad)
    {
      GenericReal<is_ad> density_sum = 0.0;
      GenericReal<is_ad> viscosity_sum = 0.0;

      for (const auto i : make_range(_n_components))
      {
        density_sum += _fp[i]->rho_from_p_T(p, T) * x[i];
        viscosity_sum += _fp[i]->mu_from_p_T(p, T) * x[i];
      }

      (*_density)[_qp] = density_sum;
      (*_viscosity)[_qp] = viscosity_sum / _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;
      Real rho_n, mu_n;

      Real density_sum = 0.0;
      Real viscosity_sum = 0.0;
      Real drho_dp_sum = 0.0;
      Real drho_dT_sum = 0.0;
      Real dmu_dp_sum = 0.0;
      Real dmu_dT_sum = 0.0;

      (*_ddensity_dX)[_qp].resize(_n_components);
      (*_dviscosity_dX)[_qp].resize(_n_components);

      // Each derivative wrt X will have a contribution from the last mass fraction
      // (which is computed as 1 - Sum(X)), so save off this value
      _fp[_n_components - 1]->rho_mu_from_p_T(
          MetaPhysicL::raw_value(p), MetaPhysicL::raw_value(T), rho_n, mu_n);
      (*_ddensity_dX)[_qp][_n_components - 1] = rho_n * dx[_n_components - 1];
      (*_dviscosity_dX)[_qp][_n_components - 1] =
          mu_n * dx[_n_components - 1] / _pressure_to_Pascals / _time_to_seconds;

      for (const auto i : make_range(_n_components))
      {
        _fp[i]->rho_mu_from_p_T(MetaPhysicL::raw_value(p),
                                MetaPhysicL::raw_value(T),
                                rho,
                                drho_dp,
                                drho_dT,
                                mu,
                                dmu_dp,
                                dmu_dT);

        density_sum += rho * MetaPhysicL::raw_value(x[i]);
        viscosity_sum += mu * MetaPhysicL::raw_value(x[i]);
        drho_dp_sum += drho_dp * MetaPhysicL::raw_value(x[i]);
        drho_dT_sum += drho_dT * MetaPhysicL::raw_value(x[i]);
        dmu_dp_sum += dmu_dp * MetaPhysicL::raw_value(x[i]);
        dmu_dT_sum += dmu_dT * MetaPhysicL::raw_value(x[i]);

        if (i < _n_components - 1)
        {
          (*_ddensity_dX)[_qp][i] = rho * dx[i] - rho_n * dx[_n_components - 1];
          (*_ddensity_dX)[_qp][_n_components - 1] -= rho * dx[i];
          (*_dviscosity_dX)[_qp][i] =
              (mu * dx[i] - mu_n * dx[_n_components - 1]) / _pressure_to_Pascals / _time_to_seconds;
          (*_dviscosity_dX)[_qp][_n_components - 1] -=
              mu * dx[i] / _pressure_to_Pascals / _time_to_seconds;
        }
      }

      (*_density)[_qp] = density_sum;
      (*_viscosity)[_qp] = viscosity_sum / _pressure_to_Pascals / _time_to_seconds;
      (*_ddensity_dp)[_qp] = drho_dp_sum * _pressure_to_Pascals;
      (*_ddensity_dT)[_qp] = drho_dT_sum;
      (*_dviscosity_dp)[_qp] = dmu_dp_sum / _time_to_seconds;
      (*_dviscosity_dT)[_qp] = dmu_dT_sum / _pressure_to_Pascals / _time_to_seconds;
    }
  }

  if (_compute_internal_energy)
  {
    if (is_ad)
    {
      GenericReal<is_ad> e_sum = 0;

      for (const auto i : make_range(_n_components))
        e_sum += _fp[i]->e_from_p_T(p, T) * x[i];

      (*_internal_energy)[_qp] = e_sum;
    }
    else
    {
      (*_dinternal_energy_dX)[_qp].resize(_n_components);
      Real e, de_dp, de_dT;
      Real e_n, de_dp_n, de_dT_n;
      GenericReal<is_ad> e_sum = 0;
      Real de_dp_sum = 0;
      Real de_dT_sum = 0;

      _fp[_n_components - 1]->e_from_p_T(
          MetaPhysicL::raw_value(p), MetaPhysicL::raw_value(T), e_n, de_dp_n, de_dT_n);
      (*_dinternal_energy_dX)[_qp][_n_components - 1] = e_n * dx[_n_components - 1];

      for (const auto i : make_range(_n_components))
      {
        _fp[i]->e_from_p_T(MetaPhysicL::raw_value(p), MetaPhysicL::raw_value(T), e, de_dp, de_dT);

        e_sum += e * x[i];
        de_dp_sum += de_dp * MetaPhysicL::raw_value(x[i]);
        de_dT_sum += de_dT * MetaPhysicL::raw_value(x[i]);

        if (i < _n_components - 1)
        {
          (*_dinternal_energy_dX)[_qp][i] = e * dx[i] - e_n * dx[_n_components - 1];
          (*_dinternal_energy_dX)[_qp][_n_components - 1] -= e * dx[i];
        }
      }

      (*_internal_energy)[_qp] = e_sum;
      (*_dinternal_energy_dp)[_qp] = de_dp_sum * _pressure_to_Pascals;
      (*_dinternal_energy_dT)[_qp] = de_dT_sum;
    }
  }

  if (_compute_enthalpy)
  {
    if (is_ad)
    {
      GenericReal<is_ad> h_sum = 0;

      for (const auto i : make_range(_n_components))
        h_sum += _fp[i]->h_from_p_T(p, T) * x[i];

      (*_enthalpy)[_qp] = h_sum;
    }
    else
    {
      Real h, dh_dp, dh_dT;
      Real h_n, dh_dp_n, dh_dT_n;
      GenericReal<is_ad> h_sum = 0;
      Real dh_dp_sum = 0;
      Real dh_dT_sum = 0;

      (*_denthalpy_dX)[_qp].resize(_n_components);

      _fp[_n_components - 1]->h_from_p_T(
          MetaPhysicL::raw_value(p), MetaPhysicL::raw_value(T), h_n, dh_dp_n, dh_dT_n);
      (*_denthalpy_dX)[_qp][_n_components - 1] = h_n * dx[_n_components - 1];

      for (unsigned int i = 0; i < _n_components; i++)
      {
        _fp[i]->h_from_p_T(MetaPhysicL::raw_value(p), MetaPhysicL::raw_value(T), h, dh_dp, dh_dT);

        h_sum += h * x[i];
        dh_dp_sum += dh_dp * MetaPhysicL::raw_value(x[i]);
        dh_dT_sum += dh_dT * MetaPhysicL::raw_value(x[i]);

        if (i < _n_components - 1)
        {
          (*_denthalpy_dX)[_qp][i] = h * dx[i] - h_n * dx[_n_components - 1];
          (*_denthalpy_dX)[_qp][_n_components - 1] -= h * dx[i];
        }
      }

      (*_enthalpy)[_qp] = h_sum;
      (*_denthalpy_dp)[_qp] = dh_dp_sum * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = dh_dT_sum;
    }
  }
}

template <bool is_ad>
std::vector<GenericReal<is_ad>>
PorousFlowMultiComponentGasMixtureTempl<is_ad>::massFractionsToMoleFractions(
    std::vector<GenericReal<is_ad>> & X)
{
  mooseAssert(X.size() == _M.size(),
              "Number of molar masses is not equal to the number of mass fractions");

  // Average molar mass of mixture
  GenericReal<is_ad> sum = 0.0;
  for (const auto i : make_range(X.size()))
    sum += X[i] / _M[i];

  const auto Mbar = 1.0 / sum;

  // Convert mass fractions to mole fractions
  std::vector<GenericReal<is_ad>> x(X.size());

  for (const auto i : make_range(X.size()))
    x[i] = X[i] / _M[i] * Mbar;

  return x;
}

template <bool is_ad>
std::vector<Real>
PorousFlowMultiComponentGasMixtureTempl<is_ad>::dMoleFraction(std::vector<GenericReal<is_ad>> & X)
{
  mooseAssert(X.size() == _M.size(),
              "Number of molar masses is not equal to the number of mass fractions");

  // Average molar mass of mixture
  Real sum = 0.0;
  for (const auto i : make_range(X.size()))
    sum += MetaPhysicL::raw_value(X[i]) / _M[i];

  const Real Mbar = 1.0 / sum;

  // Derivative of mole fraction wrt mass fractions
  std::vector<Real> dx(X.size());

  // dx[_M.size() - 1] = Mbar / _M[_M.size() - 1] - MetaPhysicL::raw_value(X[_M.size() - 1]) /
  //                                                    _M[_M.size() - 1] * (1.0 / _M[_M.size() -
  //                                                    1]) * Mbar * Mbar;

  for (const auto i : make_range(dx.size()))
  {
    dx[i] = Mbar / _M[i] - MetaPhysicL::raw_value(X[i]) / _M[i] *
                               (1.0 / _M[i] - 1.0 / _M[_M.size() - 1]) * Mbar * Mbar;
  }

  return dx;
}

template class PorousFlowMultiComponentGasMixtureTempl<false>;
template class PorousFlowMultiComponentGasMixtureTempl<true>;
