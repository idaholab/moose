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
  const ADReal p = _porepressure[_qp][_phase_num] * _pressure_to_Pascals;
  const ADReal T = _temperature[_qp] + _t_c2k;

  // Convert mass fractions (upper case) to mole fractions (lower case)
  std::vector<ADReal> X(_n_components);
  for (const auto i : make_range(_X_components))
    X[i] = (*_X[i])[_qp];

  // The final mass fraction is 1 - sum_over_mass_fractions
  ADReal total_mass_frac = 0.0;
  for (auto & Xi : X)
    total_mass_frac += Xi;

  X[_n_components - 1] = 1.0 - total_mass_frac;

  const auto x = massFractionsToMoleFractions(X);

  if (_compute_rho_mu)
  {
    // Don't need the derivatives here
    (*_density)[_qp] = MetaPhysicL::raw_value(mixtureDensity(p, T, x));
    (*_viscosity)[_qp] =
        MetaPhysicL::raw_value(mixtureViscosity(p, T, x)) / _pressure_to_Pascals / _time_to_seconds;
  }

  if (_compute_internal_energy)
  {
    // Don't need the derivatives here
    (*_internal_energy)[_qp] = MetaPhysicL::raw_value(mixtureInternalEnergy(p, T, x));
  }

  if (_compute_enthalpy)
  {
    // Don't need the derivatives here
    (*_enthalpy)[_qp] = MetaPhysicL::raw_value(mixtureEnthalpy(p, T, x));
  }
}

template <bool is_ad>
void
PorousFlowMultiComponentGasMixtureTempl<is_ad>::computeQpProperties()
{
  ADReal p = _porepressure[_qp][_phase_num] * _pressure_to_Pascals;
  ADReal T = _temperature[_qp] + _t_c2k;

  // Convert mass fractions (upper case) to mole fractions (lower case)
  std::vector<ADReal> X(_n_components);
  for (const auto i : make_range(_X_components))
    X[i] = (*_X[i])[_qp];

  // The final mass fraction is 1 - sum_over_mass_fractions
  ADReal total_mass_frac = 0.0;
  for (auto & Xi : X)
    total_mass_frac += Xi;

  X[_n_components - 1] = 1.0 - total_mass_frac;

  // We save hand-coding derivatives here to make it easier to build more complex mixing rules
  // in derived classes. So we create temporary AD primary variables even if we are not using AD
  if (!is_ad)
  {
    Moose::derivInsert(p.derivatives(), 0, 1.0);
    Moose::derivInsert(T.derivatives(), 1, 1.0);

    for (const auto i : make_range(_X_components))
    {
      Moose::derivInsert(X[i].derivatives(), 2 + i, 1.0);
      Moose::derivInsert(X[X.size() - 1].derivatives(), 2 + i, -1.0);
    }
  }

  // Compute the mole fractions
  const auto x = massFractionsToMoleFractions(X);

  if (_compute_rho_mu)
  {
    const auto density = mixtureDensity(p, T, x);
    const auto viscosity = mixtureViscosity(p, T, x);

    if constexpr (is_ad)
    {
      (*_density)[_qp] = density;
      (*_viscosity)[_qp] = viscosity / _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      (*_density)[_qp] = MetaPhysicL::raw_value(density);
      (*_viscosity)[_qp] =
          MetaPhysicL::raw_value(viscosity) / _pressure_to_Pascals / _time_to_seconds;

      // Populate the derivatives wrt primary variables
      (*_ddensity_dp)[_qp] = density.derivatives()[0] * _pressure_to_Pascals;
      (*_ddensity_dT)[_qp] = density.derivatives()[1];
      (*_dviscosity_dp)[_qp] = viscosity.derivatives()[0] / _time_to_seconds;
      (*_dviscosity_dT)[_qp] = viscosity.derivatives()[1] / _pressure_to_Pascals / _time_to_seconds;

      (*_ddensity_dX)[_qp].resize(_n_components);
      (*_dviscosity_dX)[_qp].resize(_n_components);

      for (const auto i : make_range(_X_components))
      {
        (*_ddensity_dX)[_qp][i] = density.derivatives()[i + 2];
        (*_dviscosity_dX)[_qp][i] =
            viscosity.derivatives()[i + 2] / _pressure_to_Pascals / _time_to_seconds;
      }
    }
  }

  if (_compute_internal_energy)
  {
    const auto internal_energy = mixtureInternalEnergy(p, T, x);

    if constexpr (is_ad)
    {
      (*_internal_energy)[_qp] = internal_energy;
    }
    else
    {
      (*_internal_energy)[_qp] = MetaPhysicL::raw_value(internal_energy);

      // Populate the derivatives wrt primary variables
      (*_dinternal_energy_dp)[_qp] = internal_energy.derivatives()[0] * _pressure_to_Pascals;
      (*_dinternal_energy_dT)[_qp] = internal_energy.derivatives()[1];

      (*_dinternal_energy_dX)[_qp].resize(_n_components);

      for (const auto i : make_range(_X_components))
        (*_dinternal_energy_dX)[_qp][i] = internal_energy.derivatives()[i + 2];
    }
  }

  if (_compute_enthalpy)
  {
    const auto enthalpy = mixtureEnthalpy(p, T, x);

    if constexpr (is_ad)
    {
      (*_enthalpy)[_qp] = enthalpy;
    }
    else
    {
      (*_enthalpy)[_qp] = MetaPhysicL::raw_value(enthalpy);

      // Populate the derivatives wrt primary variables
      (*_denthalpy_dp)[_qp] = enthalpy.derivatives()[0] * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = enthalpy.derivatives()[1];

      (*_denthalpy_dX)[_qp].resize(_n_components);

      for (const auto i : make_range(_X_components))
        (*_denthalpy_dX)[_qp][i] = enthalpy.derivatives()[i + 2];
    }
  }
}

template <bool is_ad>
std::vector<ADReal>
PorousFlowMultiComponentGasMixtureTempl<is_ad>::massFractionsToMoleFractions(
    std::vector<ADReal> & X)
{
  mooseAssert(X.size() == _M.size(),
              "Number of molar masses is not equal to the number of mass fractions");

  // Average molar mass of mixture
  ADReal sum = 0.0;
  for (const auto i : make_range(X.size()))
    sum += X[i] / _M[i];

  const auto Mbar = 1.0 / sum;

  // Convert mass fractions to mole fractions
  std::vector<ADReal> x(X.size());

  for (const auto i : make_range(X.size()))
    x[i] = X[i] / _M[i] * Mbar;

  return x;
}

template <bool is_ad>
ADReal
PorousFlowMultiComponentGasMixtureTempl<is_ad>::mixtureDensity(const ADReal & p,
                                                               const ADReal & T,
                                                               const std::vector<ADReal> & x)
{
  ADReal density = 0.0;

  for (const auto i : make_range(x.size()))
    density += _fp[i]->rho_from_p_T(p, T) * x[i];

  return density;
}

template <bool is_ad>
ADReal
PorousFlowMultiComponentGasMixtureTempl<is_ad>::mixtureViscosity(const ADReal & p,
                                                                 const ADReal & T,
                                                                 const std::vector<ADReal> & x)
{
  ADReal viscosity = 0.0;

  for (const auto i : make_range(x.size()))
    viscosity += _fp[i]->mu_from_p_T(p, T) * x[i];

  return viscosity;
}

template <bool is_ad>
ADReal
PorousFlowMultiComponentGasMixtureTempl<is_ad>::mixtureEnthalpy(const ADReal & p,
                                                                const ADReal & T,
                                                                const std::vector<ADReal> & x)
{
  ADReal enthalpy = 0.0;

  for (const auto i : make_range(x.size()))
    enthalpy += _fp[i]->h_from_p_T(p, T) * x[i];

  return enthalpy;
}

template <bool is_ad>
ADReal
PorousFlowMultiComponentGasMixtureTempl<is_ad>::mixtureInternalEnergy(const ADReal & p,
                                                                      const ADReal & T,
                                                                      const std::vector<ADReal> & x)
{
  ADReal internal_energy = 0.0;

  for (const auto i : make_range(x.size()))
    internal_energy += _fp[i]->e_from_p_T(p, T) * x[i];

  return internal_energy;
}

template class PorousFlowMultiComponentGasMixtureTempl<false>;
template class PorousFlowMultiComponentGasMixtureTempl<true>;
