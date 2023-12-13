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
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp", "fluid component names in order (component 0 to n-1)");
  params.addRequiredCoupledVar(
      "x",
      "mass fraction variables of component 0 to n-2 of the specifed phase (The last component "
      "mass fraction will be automatically computed)");
  params.addClassDescription(
      "This Material calculates the mixture fluid properties at the quadpoints or nodes "
      "for a multiple components fluid based on their mass fraction a");
  return params;
}
template <bool is_ad>
PorousFlowMultiComponentGasMixtureTempl<is_ad>::PorousFlowMultiComponentGasMixtureTempl(
    const InputParameters & parameters)
  : PorousFlowMultiComponentFluidBaseTempl<is_ad>(parameters),
    _fp_names(this->template getParam<std::vector<UserObjectName>>("fp")),
    _n_components(_fp_names.size()),
    _x_components(this->coupledComponents("x"))

{
  if (_n_components < 2)
    mooseError("PorousFlowMultiComponentGasMixture: This material requires at least two components "
               "in the specified phase, ",
               " and the number of entered components is ",
               _n_components);

  if (_x_components != (_n_components - 1))
    mooseError(
        "x: ",
        "The number of mass fraction variable must be equal to the number of fluid components-1 (",
        _n_components - 1,
        ")");

  _fp.resize(_n_components);

  for (unsigned int i = 0; i < _n_components; i++)
  {
    _fp[i] = &this->template getUserObjectByName<SinglePhaseFluidProperties>(_fp_names[i]);
  }

  _x.resize(_x_components);

  for (unsigned int i = 0; i < _x_components; i++)
  {
    _x[i] = (_nodal_material ? &this->template coupledGenericDofValue<is_ad>("x", i)
                             : &this->template coupledGenericValue<is_ad>("x", i));
  }
}

template <bool is_ad>
void
PorousFlowMultiComponentGasMixtureTempl<is_ad>::initQpStatefulProperties()
{
  if (_compute_rho_mu)
  {
    GenericReal<is_ad> density_sum = 0;
    GenericReal<is_ad> viscosity_sum = 0;
    GenericReal<is_ad> total_mass_frac = 0;

    for (unsigned int i = 0; i < _n_components; i++)
    {
      if (i == _n_components - 1)
      {
        density_sum += _fp[i]->rho_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                            _temperature[_qp] + _t_c2k) *
                       (1 - total_mass_frac);
        viscosity_sum += _fp[i]->mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                             _temperature[_qp] + _t_c2k) *
                         (1 - total_mass_frac);
      }
      else
      {
        density_sum += _fp[i]->rho_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                            _temperature[_qp] + _t_c2k) *
                       (*_x[i])[_qp];
        viscosity_sum += _fp[i]->mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                             _temperature[_qp] + _t_c2k) *
                         (*_x[i])[_qp];
        total_mass_frac += (*_x[i])[_qp];
      }
    }
    (*_density)[_qp] = density_sum;
    (*_viscosity)[_qp] = viscosity_sum / _pressure_to_Pascals / _time_to_seconds;
  }

  if (_compute_internal_energy)
  {
    GenericReal<is_ad> e_sum = 0;
    GenericReal<is_ad> total_mass_frac = 0;

    for (unsigned int i = 0; i < _n_components; i++)
    {
      if (i == _n_components - 1)
      {
        e_sum += _fp[i]->e_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                    _temperature[_qp] + _t_c2k) *
                 (1 - total_mass_frac);
      }
      else
      {
        e_sum += _fp[i]->e_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                    _temperature[_qp] + _t_c2k) *
                 (*_x[i])[_qp];
        total_mass_frac += (*_x[i])[_qp];
      }
    }
    (*_internal_energy)[_qp] = e_sum;
  }

  if (_compute_enthalpy)
  {
    GenericReal<is_ad> h_sum = 0;
    GenericReal<is_ad> total_mass_frac = 0;

    for (unsigned int i = 0; i < _n_components; i++)
    {
      if (i == _n_components - 1)
      {
        h_sum += _fp[i]->h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                    _temperature[_qp] + _t_c2k) *
                 (1 - total_mass_frac);
      }
      else
      {
        h_sum += _fp[i]->h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                    _temperature[_qp] + _t_c2k) *
                 (*_x[i])[_qp];
        total_mass_frac += (*_x[i])[_qp];
      }
    }
    (*_enthalpy)[_qp] = h_sum;
  }
}

template <bool is_ad>
void
PorousFlowMultiComponentGasMixtureTempl<is_ad>::computeQpProperties()
{
  if (_compute_rho_mu)
  {
    if (is_ad)
    {
      GenericReal<is_ad> density_sum = 0;
      GenericReal<is_ad> viscosity_sum = 0;
      GenericReal<is_ad> total_mass_frac = 0;

      for (unsigned int i = 0; i < _n_components; i++)
        if (i == _n_components - 1)
        {
          density_sum += _fp[i]->rho_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                              _temperature[_qp] + _t_c2k) *
                         (1 - total_mass_frac);

          viscosity_sum +=
              _fp[i]->mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                  _temperature[_qp] + _t_c2k) *
              (1 - total_mass_frac);
        }
        else
        {
          density_sum += _fp[i]->rho_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                              _temperature[_qp] + _t_c2k) *
                         (*_x[i])[_qp];
          viscosity_sum +=
              _fp[i]->mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                  _temperature[_qp] + _t_c2k) *
              (*_x[i])[_qp];
          total_mass_frac += (*_x[i])[_qp];
        }
      (*_density)[_qp] = density_sum;
      (*_viscosity)[_qp] = viscosity_sum / _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      Real rho_end, drho_dp_end, drho_dT_end, mu_end, dmu_dp_end, dmu_dT_end;
      Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;
      GenericReal<is_ad> total_mass_frac = 0;
      GenericReal<is_ad> rho_sum = 0;
      GenericReal<is_ad> mu_sum = 0;
      Real drho_dp_sum = 0;
      Real drho_dT_sum = 0;
      Real dmu_dp_sum = 0;
      Real dmu_dT_sum = 0;
      (*_ddensity_dX)[_qp].resize(_x_components);
      (*_dviscosity_dX)[_qp].resize(_x_components);

      _fp[_n_components - 1]->rho_mu_from_p_T(
          MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) * _pressure_to_Pascals,
          MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
          rho_end,
          drho_dp_end,
          drho_dT_end,
          mu_end,
          dmu_dp_end,
          dmu_dT_end);

      for (unsigned int i = 0; i < _n_components; i++)
      {
        _fp[i]->rho_mu_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                    _pressure_to_Pascals,
                                MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                                rho,
                                drho_dp,
                                drho_dT,
                                mu,
                                dmu_dp,
                                dmu_dT);
        if (i == _n_components - 1)
        {
          rho_sum += rho * (1 - total_mass_frac);
          mu_sum += (mu * (1 - total_mass_frac));
          drho_dp_sum += (drho_dp * MetaPhysicL::raw_value((1 - total_mass_frac)));
          drho_dT_sum += drho_dT * MetaPhysicL::raw_value((1 - total_mass_frac));
          dmu_dp_sum += (dmu_dp * MetaPhysicL::raw_value((1 - total_mass_frac)));
          dmu_dT_sum = (dmu_dT * MetaPhysicL::raw_value((1 - total_mass_frac)));
        }
        else
        {
          (*_ddensity_dX)[_qp][i] = rho - rho_end;
          (*_dviscosity_dX)[_qp][i] = (mu - mu_end) / _pressure_to_Pascals / _time_to_seconds;
          rho_sum += rho * (*_x[i])[_qp];
          mu_sum += mu * (*_x[i])[_qp];
          drho_dp_sum += drho_dp * MetaPhysicL::raw_value((*_x[i])[_qp]);
          drho_dT_sum += drho_dT * MetaPhysicL::raw_value((*_x[i])[_qp]);
          dmu_dp_sum += dmu_dp * MetaPhysicL::raw_value((*_x[i])[_qp]);
          dmu_dT_sum += dmu_dT * MetaPhysicL::raw_value((*_x[i])[_qp]);
          total_mass_frac += (*_x[i])[_qp];
        }
      }
      (*_density)[_qp] = rho_sum;
      (*_viscosity)[_qp] = mu_sum / _pressure_to_Pascals / _time_to_seconds;
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
      GenericReal<is_ad> total_mass_frac = 0;
      for (unsigned int i = 0; i < _n_components; i++)
      {
        if (i == _n_components - 1)
        {
          e_sum += _fp[i]->e_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                      _temperature[_qp] + _t_c2k) *
                   (1 - total_mass_frac);
        }
        else
        {
          e_sum += _fp[i]->e_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                      _temperature[_qp] + _t_c2k) *
                   (*_x[i])[_qp];
          total_mass_frac += (*_x[i])[_qp];
        }
      }
      (*_internal_energy)[_qp] = e_sum;
    }
    else
    {
      (*_dinternal_energy_dX)[_qp].resize(_x_components);
      Real e, de_dp, de_dT;
      Real e_end, de_dp_end, de_dT_end;
      GenericReal<is_ad> total_mass_frac = 0;
      GenericReal<is_ad> e_sum = 0;
      Real de_dp_sum = 0;
      Real de_dT_sum = 0;

      _fp[_n_components - 1]->e_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                             _pressure_to_Pascals,
                                         MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                                         e_end,
                                         de_dp_end,
                                         de_dT_end);

      for (unsigned int i = 0; i < _n_components; i++)
      {
        _fp[i]->e_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                               _pressure_to_Pascals,
                           MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                           e,
                           de_dp,
                           de_dT);
        if (i == _n_components - 1)
        {
          e_sum += e * (1 - total_mass_frac);
          de_dp_sum += (de_dp * MetaPhysicL::raw_value((1 - total_mass_frac)));
          de_dT_sum += de_dT * MetaPhysicL::raw_value((1 - total_mass_frac));
        }
        else
        {
          (*_dinternal_energy_dX)[_qp][i] = e - e_end;
          e_sum += e * (*_x[i])[_qp];
          de_dp_sum += de_dp * MetaPhysicL::raw_value((*_x[i])[_qp]);
          de_dT_sum += de_dT * MetaPhysicL::raw_value((*_x[i])[_qp]);
          total_mass_frac += (*_x[i])[_qp];
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
      GenericReal<is_ad> total_mass_frac = 0;
      for (unsigned int i = 0; i < _n_components; i++)
      {
        if (i == _n_components - 1)
        {
          h_sum += _fp[i]->h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                      _temperature[_qp] + _t_c2k) *
                   (1 - total_mass_frac);
        }
        else
        {
          h_sum += _fp[i]->h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                      _temperature[_qp] + _t_c2k) *
                   (*_x[i])[_qp];
          total_mass_frac += (*_x[i])[_qp];
        }
      }
      (*_enthalpy)[_qp] = h_sum;
    }
    else
    {
      (*_denthalpy_dX)[_qp].resize(_x_components);
      Real h, dh_dp, dh_dT;
      Real h_end, dh_dp_end, dh_dT_end;
      GenericReal<is_ad> total_mass_frac = 0;
      GenericReal<is_ad> h_sum = 0;
      Real dh_dp_sum = 0;
      Real dh_dT_sum = 0;

      _fp[_n_components - 1]->h_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                             _pressure_to_Pascals,
                                         MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                                         h_end,
                                         dh_dp_end,
                                         dh_dT_end);

      for (unsigned int i = 0; i < _n_components; i++)
      {
        _fp[i]->h_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                               _pressure_to_Pascals,
                           MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                           h,
                           dh_dp,
                           dh_dT);
        if (i == _n_components - 1)
        {
          h_sum += h * (1 - total_mass_frac);
          dh_dp_sum += dh_dp * MetaPhysicL::raw_value((1 - total_mass_frac));
          dh_dT_sum += dh_dT * MetaPhysicL::raw_value((1 - total_mass_frac));
        }
        else
        {
          (*_denthalpy_dX)[_qp][i] = h - h_end;
          h_sum += h * (*_x[i])[_qp];
          dh_dp_sum += dh_dp * MetaPhysicL::raw_value((*_x[i])[_qp]);
          dh_dT_sum += dh_dT * MetaPhysicL::raw_value((*_x[i])[_qp]);
          total_mass_frac += (*_x[i])[_qp];
        }
      }
      (*_enthalpy)[_qp] = h_sum;
      (*_denthalpy_dp)[_qp] = dh_dp_sum * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = dh_dT_sum;
    }
  }
}
template class PorousFlowMultiComponentGasMixtureTempl<false>;
template class PorousFlowMultiComponentGasMixtureTempl<true>;
