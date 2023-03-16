//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectrochemicalSinteringMaterial.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", ElectrochemicalSinteringMaterial);

InputParameters
ElectrochemicalSinteringMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Includes switching and thermodynamic properties for the grand "
                             "potential sintering model coupled with electrochemistry");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of order parameters that describe solid phase");
  params.addRequiredCoupledVar("chemical_potentials",
                               "The name of the chemical potential variables for defects");
  params.addRequiredCoupledVar("void_op", "The name of the void phase order parameter");
  params.addRequiredCoupledVar("Temperature", "Name of the temperature variable with units of K");
  params.addRequiredCoupledVar("electric_potential",
                               "Name of the electric potential variable with units of V");
  params.addParam<std::vector<MaterialPropertyName>>(
      "solid_energy_coefficients",
      "Vector of parabolic solid energy coefficients (energy*volume) for "
      "each defect species. Only used for parabolic energy. Place in same order as "
      "chemical_potentials!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "void_energy_coefficients",
      "Vector of parabolic void energy coefficients (energy*volume) for each defect species. Place "
      "in same order as chemical_potentials!");
  params.addParam<Real>("surface_energy", 19.7, "Surface energy in units of problem (energy/area)");
  params.addParam<Real>(
      "grainboundary_energy", 9.86, "Grain boundary energy in units of problem (energy/area)");
  params.addParam<Real>("int_width", 1, "Interface width in units of problem (length)");
  params.addRangeCheckedParam<Real>(
      "surface_switch_value",
      0.3,
      "surface_switch_value >= 0 & surface_switch_value <= 1.0",
      "Value between 0 and 1 that determines when the interface begins to switch "
      "from surface to GB. Small values give less error while large values "
      "converge better.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "min_vacancy_concentrations_solid",
      "Vector of names of materials that determine the minimum in energy wrt defect concentrations "
      "in the solid phase. Place in same order as chemical_potentials!");
  params.addRequiredParam<std::vector<Real>>("min_vacancy_concentrations_void",
                                             "Vector of minima in energy wrt defect concentrations "
                                             "in the void phase. Place in same order as "
                                             "chemical_potentials!");
  MooseEnum solid_energy_model("PARABOLIC DILUTE IDEAL", "PARABOLIC");
  params.addParam<MooseEnum>("solid_energy_model",
                             solid_energy_model,
                             "Type of energy function to use for the solid phase.");
  params.addRequiredParam<std::vector<int>>(
      "defect_charges",
      "Vector of charges of defect species. Place in same order as chemical_potentials!");
  params.addRequiredParam<Real>("solid_relative_permittivity",
                                "Solid phase relative permittivity (dimensionless)");
  params.addParam<Real>("voltage_scale", 1, "Voltage scale (default is for voltage in V)");
  return params;
}

ElectrochemicalSinteringMaterial::ElectrochemicalSinteringMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _neta(coupledComponents("etas")),
    _eta(_neta),
    _eta_name(_neta),
    _ndefects(coupledComponents("chemical_potentials")),
    _w(coupledValues("chemical_potentials")),
    _w_name(_ndefects),
    _phi(coupledValue("void_op")),
    _phi_name(coupledName("void_op", 0)),
    _ns_min_names(getParam<std::vector<MaterialPropertyName>>("min_vacancy_concentrations_solid")),
    _ns_min(_ndefects),
    _dns_min(_ndefects),
    _d2ns_min(_ndefects),
    _temp(coupledValue("Temperature")),
    _v(coupledValue("electric_potential")),
    _grad_V(coupledGradient("electric_potential")),
    _kv_names(getParam<std::vector<MaterialPropertyName>>("void_energy_coefficients")),
    _kv(_ndefects),
    _ks_names(getParam<std::vector<MaterialPropertyName>>("solid_energy_coefficients")),
    _ks(_ndefects),
    _hv(declareProperty<Real>("hv")),
    _dhv(declarePropertyDerivative<Real>("hv", _phi_name)),
    _d2hv(declarePropertyDerivative<Real>("hv", _phi_name, _phi_name)),
    _hs(declareProperty<Real>("hs")),
    _dhs(declarePropertyDerivative<Real>("hs", _phi_name)),
    _d2hs(declarePropertyDerivative<Real>("hs", _phi_name, _phi_name)),
    _omegav(declareProperty<Real>("omegav")),
    _domegavdw(_ndefects),
    _d2omegavdw2(_ndefects),
    _omegas(declareProperty<Real>("omegas")),
    _domegasdw(_ndefects),
    _d2omegasdw2(_ndefects),
    _domegasdeta(_neta),
    _d2omegasdwdeta(_ndefects),
    _d2omegasdetadeta(_neta),
    _mu(declareProperty<Real>("mu")),
    _dmu(declarePropertyDerivative<Real>("mu", _phi_name)),
    _d2mu(declarePropertyDerivative<Real>("mu", _phi_name, _phi_name)),
    _kappa(declareProperty<Real>("kappa")),
    _dkappa(declarePropertyDerivative<Real>("kappa", _phi_name)),
    _d2kappa(declarePropertyDerivative<Real>("kappa", _phi_name, _phi_name)),
    _gamma(declareProperty<Real>("gamma")),
    _sigma_s(getParam<Real>("surface_energy")),
    _sigma_gb(getParam<Real>("grainboundary_energy")),
    _int_width(getParam<Real>("int_width")),
    _switch(getParam<Real>("surface_switch_value")),
    _solid_energy(getParam<MooseEnum>("solid_energy_model")),
    _mu_s(6.0 * _sigma_s / _int_width),
    _mu_gb(6.0 * _sigma_gb / _int_width),
    _kappa_s(0.75 * _sigma_s * _int_width),
    _kappa_gb(0.75 * _sigma_gb * _int_width),
    _kB(8.617343e-5), // eV/K
    _e(1.0),          // to put energy units in eV
    _z(getParam<std::vector<int>>("defect_charges")),
    _v_scale(getParam<Real>("voltage_scale")),
    _eps_r(getParam<Real>("solid_relative_permittivity")),
    _nv_min(getParam<std::vector<Real>>("min_vacancy_concentrations_void"))
{
  if (_ndefects != _z.size())
    paramError("defect_charges", "Need to pass in as many defect_charges as chemical_potentials");
  if (_ndefects != _ns_min_names.size())
    paramError("min_vacancy_concentrations_solid",
               "Need to pass in as many min_vacancy_concentrations_solid as chemical_potentials");
  if (_ndefects != _nv_min.size())
    paramError("min_vacancy_concentrations_void",
               "Need to pass in as many min_vacancy_concentrations_void as chemical_potentials");
  if (_ndefects != _kv_names.size())
    paramError("void_energy_coefficients",
               "Need to pass in as many void_energy_coefficients as chemical_potentials");
  if (_ndefects != _ks_names.size())
    paramError("solid_energy_coefficients",
               "Need to pass in as many solid_energy_coefficients as chemical_potentials");

  for (unsigned int i = 0; i < _ndefects; ++i)
  {
    _w_name[i] = coupledName("chemical_potentials", i);
    _ns_min[i] = &getMaterialPropertyByName<Real>(_ns_min_names[i]);
    _dns_min[i].resize(_neta);
    _d2ns_min[i].resize(_neta);
    _d2omegasdwdeta[i].resize(_neta);
    _kv[i] = &getMaterialPropertyByName<Real>(_kv_names[i]);
    _ks[i] = &getMaterialPropertyByName<Real>(_ks_names[i]);
    _domegavdw[i] = &declarePropertyDerivative<Real>("omegav", _w_name[i]);
    _d2omegavdw2[i] = &declarePropertyDerivative<Real>("omegav", _w_name[i], _w_name[i]);
    _domegasdw[i] = &declarePropertyDerivative<Real>("omegas", _w_name[i]);
    _d2omegasdw2[i] = &declarePropertyDerivative<Real>("omegas", _w_name[i], _w_name[i]);
    for (unsigned int j = 0; j < _neta; ++j)
    {
      _dns_min[i][j] = &getMaterialPropertyDerivativeByName<Real>(_ns_min_names[i], _eta_name[j]);
      _d2ns_min[i][j].resize(_neta);

      for (unsigned int k = 0; k < _neta; ++k)
        _d2ns_min[i][j][k] = &getMaterialPropertyDerivativeByName<Real>(
            _ns_min_names[i], _eta_name[j], _eta_name[k]);
    }
  }

  for (unsigned int i = 0; i < _neta; ++i)
  {
    _eta[i] = &coupledValue("etas", i);
    _eta_name[i] = coupledName("etas", i);
    _domegasdeta[i] = &declarePropertyDerivative<Real>("omegas", _eta_name[i]);
    _d2omegasdetadeta[i].resize(_neta);

    for (unsigned int j = 0; j <= i; ++j)
      _d2omegasdetadeta[j][i] = _d2omegasdetadeta[i][j] =
          &declarePropertyDerivative<Real>("omegas", _eta_name[j], _eta_name[i]);
  }

  for (unsigned int i = 0; i < _ndefects; ++i)
    for (unsigned int j = 0; j < _neta; ++j)
      _d2omegasdwdeta[i][j] = &declarePropertyDerivative<Real>("omegas", _w_name[i], _eta_name[j]);
}

void
ElectrochemicalSinteringMaterial::computeQpProperties()
{
  // Calculate phase switching functions
  _hv[_qp] = 0.0;
  _dhv[_qp] = 0.0;
  _d2hv[_qp] = 0.0;

  if (_phi[_qp] >= 1.0)
    _hv[_qp] = 1.0;
  else if (_phi[_qp] > 0.0)
  {
    _hv[_qp] = _phi[_qp] * _phi[_qp] * _phi[_qp] * (10.0 + _phi[_qp] * (-15.0 + _phi[_qp] * 6.0));
    _dhv[_qp] = 30.0 * _phi[_qp] * _phi[_qp] * (_phi[_qp] - 1.0) * (_phi[_qp] - 1.0);
    _d2hv[_qp] = 60.0 * _phi[_qp] * (2.0 * _phi[_qp] - 1.0) * (_phi[_qp] - 1.0);
  }

  _hs[_qp] = 1.0 - _hv[_qp];
  _dhs[_qp] = -_dhv[_qp];
  _d2hs[_qp] = -_d2hv[_qp];

  // Calculate interface switching function
  Real phi = _phi[_qp] / _switch;
  Real f = 0.0;
  Real df = 0.0;
  Real d2f = 0.0;
  if (phi >= 1.0)
    f = 1.0;
  else if (phi > 0.0)
  {
    f = phi * phi * phi * (10.0 + phi * (-15.0 + phi * 6.0));
    df = 30.0 / _switch * phi * phi * (phi - 1.0) * (phi - 1.0);
    d2f = 60.0 * phi / (_switch * _switch) * (2.0 * phi - 1.0) * (phi - 1.0);
  }

  // Permittivity in the void phase (permittivity of free space)
  Real eps_0 = 5.52e-2 * _v_scale * _v_scale; // units eV/V^2/nm, change with V_scale if needed

  // Calculate grand potential of void phase
  Real sum_omega_v = 0.0;
  for (unsigned int i = 0; i < _ndefects; ++i)
    sum_omega_v += -0.5 / (*_kv[i])[_qp] * ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) *
                       ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) -
                   _nv_min[i] * ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale);

  _omegav[_qp] = sum_omega_v - 0.5 * eps_0 * _grad_V[_qp] * _grad_V[_qp];

  // Calculate derivatives of grand potential wrt chemical potential
  for (unsigned int i = 0; i < _ndefects; ++i)
  {
    (*_domegavdw[i])[_qp] =
        -((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) / (*_kv[i])[_qp] - _nv_min[i];
    (*_d2omegavdw2[i])[_qp] = -1.0 / (*_kv[i])[_qp];
  }

  // Calculate solid phase density and potential
  switch (_solid_energy)
  {
    case 0: // PARABOLIC
    {
      // Calculate grand potential of solid phase and derivatives wrt chemical potentials
      Real sum_omega_s = 0.0;
      for (unsigned int i = 0; i < _ndefects; ++i)
      {
        sum_omega_s += -0.5 / (*_ks[i])[_qp] * ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) *
                           ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) -
                       (*_ns_min[i])[_qp] * ((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale);
        (*_domegasdw[i])[_qp] =
            -((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) / (*_ks[i])[_qp] -
            (*_ns_min[i])[_qp];
        (*_d2omegasdw2[i])[_qp] = -1.0 / (*_ks[i])[_qp];
      }

      _omegas[_qp] = sum_omega_s - 0.5 * _eps_r * eps_0 * _grad_V[_qp] * _grad_V[_qp];

      // Calculate derivatives of grand potential wrt order parameters and mixed derivatives
      for (unsigned int i = 0; i < _neta; ++i)
      {
        Real sum_domegasdeta = 0.0;
        for (unsigned int j = 0; j < _ndefects; ++j)
        {
          sum_domegasdeta +=
              -(*_dns_min[j][i])[_qp] * ((*_w[j])[_qp] - _z[j] * _e * _v[_qp] * _v_scale);
          (*_d2omegasdwdeta[j][i])[_qp] = -(*_dns_min[j][i])[_qp];
        }
        (*_domegasdeta[i])[_qp] = sum_domegasdeta;

        for (unsigned int j = i; j < _neta; ++j)
        {
          Real sum_d2omegadeta2 = 0.0;
          for (unsigned int k = 0; k < _ndefects; ++k)
            sum_d2omegadeta2 +=
                -(*_d2ns_min[k][i][j])[_qp] * ((*_w[k])[_qp] - _z[k] * _e * _v[_qp] * _v_scale);

          (*_d2omegasdetadeta[i][j])[_qp] = (*_d2omegasdetadeta[j][i])[_qp] = sum_d2omegadeta2;
        }
      }
      break;
    }       // case 0; // PARABOLIC
    case 1: // DILUTE
    {
      // Calculate grand potential of solid phase and derivatives wrt chemical potentials
      Real sum_omega_s = 0.0;
      for (unsigned int i = 0; i < _ndefects; ++i)
      {
        Real n_exp = std::exp(((*_w[i])[_qp] - _z[i] * _e * _v[_qp] * _v_scale) / _kB / _temp[_qp]);
        sum_omega_s += -_kB * _temp[_qp] * (*_ns_min[i])[_qp] * n_exp;
        (*_domegasdw[i])[_qp] = -(*_ns_min[i])[_qp] * n_exp;
        (*_d2omegasdw2[i])[_qp] = -(*_ns_min[i])[_qp] * n_exp / _kB / _temp[_qp];
      }

      _omegas[_qp] = sum_omega_s - 0.5 * _eps_r * eps_0 * _grad_V[_qp] * _grad_V[_qp];

      // Calculate derivatives of grand potential wrt order parameters and mixed derivatives
      for (unsigned int i = 0; i < _neta; ++i)
      {
        Real sum_domegasdeta = 0.0;
        for (unsigned int j = 0; j < _ndefects; ++j)
        {
          Real n_exp =
              std::exp(((*_w[j])[_qp] - _z[j] * _e * _v[_qp] * _v_scale) / _kB / _temp[_qp]);
          sum_domegasdeta += -_kB / _temp[_qp] * (*_dns_min[j][i])[_qp] * n_exp;
          (*_d2omegasdwdeta[j][i])[_qp] = -(*_dns_min[j][i])[_qp] * n_exp;
        }
        (*_domegasdeta[i])[_qp] = sum_domegasdeta;

        for (unsigned int j = i; j < _neta; ++j)
        {
          Real sum_d2omegadeta2 = 0.0;
          for (unsigned int k = 0; k < _ndefects; ++k)
          {
            Real n_exp =
                std::exp(((*_w[k])[_qp] - _z[k] * _e * _v[_qp] * _v_scale) / _kB / _temp[_qp]);
            sum_d2omegadeta2 += -_kB / _temp[_qp] * (*_d2ns_min[k][i][j])[_qp] * n_exp;
          }

          (*_d2omegasdetadeta[i][j])[_qp] = (*_d2omegasdetadeta[j][i])[_qp] = sum_d2omegadeta2;
        }
      }

      break;
    }       // case 1: // DILUTE
    case 2: // IDEAL
    {
      mooseError(
          "Ideal solution in solid is not yet supported in ElectrochemicalSinteringMaterial");
      break;
    } // case 2: // IDEAL
  }   // switch (_solid_energy)

  // thermodynamic parameters
  _mu[_qp] = _mu_gb + (_mu_s - _mu_gb) * f;
  _kappa[_qp] = _kappa_gb + (_kappa_s - _kappa_gb) * f;
  _dmu[_qp] = (_mu_s - _mu_gb) * df;
  _dkappa[_qp] = (_kappa_s - _kappa_gb) * df;
  _d2mu[_qp] = (_mu_s - _mu_gb) * d2f;
  _d2kappa[_qp] = (_kappa_s - _kappa_gb) * d2f;
  _gamma[_qp] = 1.5;
} // void ElectrochemicalSinteringMaterial::computeQpProperties()
