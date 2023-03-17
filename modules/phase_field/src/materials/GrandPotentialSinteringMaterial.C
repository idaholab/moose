//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrandPotentialSinteringMaterial.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", GrandPotentialSinteringMaterial);

InputParameters
GrandPotentialSinteringMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Includes switching and thermodynamic properties for the grand potential sintering model");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of order parameters that describe solid phase");
  params.addRequiredCoupledVar("chemical_potential", "The name of the chemical potential variable");
  params.addRequiredCoupledVar("void_op", "The name of the void phase order parameter");
  params.addRequiredCoupledVar("Temperature", "Name of the temperature variable with units of K");
  params.addParam<MaterialPropertyName>(
      "solid_energy_coefficient",
      1.0,
      "Parabolic solid energy coefficient (energy/volume). Only used for parabolic energy.");
  params.addRequiredParam<MaterialPropertyName>(
      "void_energy_coefficient", "Parabolic void energy coefficient (energy/volume)");
  params.addParam<Real>("surface_energy", 19.7, "Surface energy in units of problem (energy/area)");
  params.addParam<Real>(
      "grainboundary_energy", 9.86, "Grain boundary energy in units of problem (energy/area)");
  params.addParam<Real>("int_width", 1, "Interface width in units of problem (length)");
  params.addParam<Real>("surface_switch_value",
                        0.3,
                        "Value between 0 and 1 that determines when the interface begins to switch "
                        "from surface to GB. Small values give less error while large values "
                        "converge better.");
  params.addRequiredParam<MaterialPropertyName>(
      "equilibrium_vacancy_concentration",
      "Name of material that determines the equilibrium vacancy concentration in the solid phase");
  params.addParam<Real>("atomic_volume", 0.04092, "Atomic volume of material");
  MooseEnum solid_energy_model("PARABOLIC DILUTE IDEAL", "PARABOLIC");
  params.addParam<MooseEnum>("solid_energy_model",
                             solid_energy_model,
                             "Type of energy function to use for the solid phase.");
  params.addParam<bool>(
      "mass_conservation", false, "imposing strict mass conservation formulation");
  return params;
}

GrandPotentialSinteringMaterial::GrandPotentialSinteringMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _neta(coupledComponents("etas")),
    _eta(_neta),
    _eta_name(_neta),
    _w(coupledValue("chemical_potential")),
    _w_name(coupledName("chemical_potential", 0)),
    _phi(coupledValue("void_op")),
    _phi_name(coupledName("void_op", 0)),
    _cs_eq_name(getParam<MaterialPropertyName>("equilibrium_vacancy_concentration")),
    _cs_eq(getMaterialProperty<Real>(_cs_eq_name)),
    _dcs_eq(_neta),
    _d2cs_eq(_neta),
    _T(coupledValue("Temperature")),
    _kv(getMaterialProperty<Real>("void_energy_coefficient")),
    _ks(getMaterialProperty<Real>("solid_energy_coefficient")),
    _hv(declareProperty<Real>("hv")),
    _dhv(declarePropertyDerivative<Real>("hv", _phi_name)),
    _d2hv(declarePropertyDerivative<Real>("hv", _phi_name, _phi_name)),
    _hs(declareProperty<Real>("hs")),
    _dhs(declarePropertyDerivative<Real>("hs", _phi_name)),
    _d2hs(declarePropertyDerivative<Real>("hs", _phi_name, _phi_name)),
    _chi(declareProperty<Real>("chi")),
    _dchidphi(declarePropertyDerivative<Real>("chi", _phi_name)),
    _dchidw(declarePropertyDerivative<Real>("chi", _w_name)),
    _d2chidphi2(declarePropertyDerivative<Real>("chi", _phi_name, _phi_name)),
    _d2chidw2(declarePropertyDerivative<Real>("chi", _w_name, _w_name)),
    _d2chidphidw(declarePropertyDerivative<Real>("chi", _phi_name, _w_name)),
    _rhov(declareProperty<Real>("rhov")),
    _drhovdw(declarePropertyDerivative<Real>("rhov", _w_name)),
    _rhos(declareProperty<Real>("rhos")),
    _drhosdw(declarePropertyDerivative<Real>("rhos", _w_name)),
    _d2rhosdw2(declarePropertyDerivative<Real>("rhos", _w_name, _w_name)),
    _drhos(_neta),
    _d2rhosdwdeta(_neta),
    _d2rhos(_neta),
    _omegav(declareProperty<Real>("omegav")),
    _domegavdw(declarePropertyDerivative<Real>("omegav", _w_name)),
    _d2omegavdw2(declarePropertyDerivative<Real>("omegav", _w_name, _w_name)),
    _omegas(declareProperty<Real>("omegas")),
    _domegasdw(declarePropertyDerivative<Real>("omegas", _w_name)),
    _d2omegasdw2(declarePropertyDerivative<Real>("omegas", _w_name, _w_name)),
    _domegasdeta(_neta),
    _d2omegasdwdeta(_neta),
    _d2omegasdetadeta(_neta),
    _mu(declareProperty<Real>("mu")),
    _dmu(declarePropertyDerivative<Real>("mu", _phi_name)),
    _d2mu(declarePropertyDerivative<Real>("mu", _phi_name, _phi_name)),
    _kappa(declareProperty<Real>("kappa")),
    _dkappa(declarePropertyDerivative<Real>("kappa", _phi_name)),
    _d2kappa(declarePropertyDerivative<Real>("kappa", _phi_name, _phi_name)),
    _gamma(declareProperty<Real>("gamma")),
    _hv_c_min(declareProperty<Real>("hv_c_min")),
    _hs_c_min(declareProperty<Real>("hs_c_min")),
    _hv_over_kVa(declareProperty<Real>("hv_over_kVa")),
    _hs_over_kVa(declareProperty<Real>("hs_over_kVa")),

    _sigma_s(getParam<Real>("surface_energy")),
    _sigma_gb(getParam<Real>("grainboundary_energy")),
    _int_width(getParam<Real>("int_width")),
    _switch(getParam<Real>("surface_switch_value")),
    _Va(getParam<Real>("atomic_volume")),
    _solid_energy(getParam<MooseEnum>("solid_energy_model")),
    _mu_s(6.0 * _sigma_s / _int_width),
    _mu_gb(6.0 * _sigma_gb / _int_width),
    _kappa_s(0.75 * _sigma_s * _int_width),
    _kappa_gb(0.75 * _sigma_gb * _int_width),
    _kB(8.617343e-5), // eV/K
    _mass_conservation(getParam<bool>("mass_conservation"))
{
  if ((_switch > 1.0) || (_switch < 0.0))
    mooseError("GrandPotentialSinteringMaterial: surface_switch_value should be between 0 and 1");

  if (_mass_conservation && _solid_energy > 0)
    mooseError("GrandPotentialSinteringMaterial: strict mass conservation is currently only "
               "applicable to parabolic free energy");

  for (unsigned int i = 0; i < _neta; ++i)
  {
    _eta[i] = &coupledValue("etas", i);
    _eta_name[i] = coupledName("etas", i);
    _dcs_eq[i] = &getMaterialPropertyDerivativeByName<Real>(_cs_eq_name, _eta_name[i]);
    _d2cs_eq[i].resize(_neta);
    _drhos[i] = &declarePropertyDerivative<Real>("rhos", _eta_name[i]);
    _d2rhos[i].resize(_neta);
    _d2rhosdwdeta[i] = &declarePropertyDerivative<Real>("rhos", _w_name, _eta_name[i]);
    _domegasdeta[i] = &declarePropertyDerivative<Real>("omegas", _eta_name[i]);
    _d2omegasdwdeta[i] = &declarePropertyDerivative<Real>("omegas", _w_name, _eta_name[i]);
    _d2omegasdetadeta[i].resize(_neta);

    for (unsigned int j = 0; j <= i; ++j)
    {
      _d2cs_eq[j][i] =
          &getMaterialPropertyDerivativeByName<Real>(_cs_eq_name, _eta_name[j], _eta_name[i]);
      _d2rhos[j][i] = &declarePropertyDerivative<Real>("rhos", _eta_name[j], _eta_name[i]);
      _d2omegasdetadeta[j][i] =
          &declarePropertyDerivative<Real>("omegas", _eta_name[j], _eta_name[i]);
    }
  }
}

void
GrandPotentialSinteringMaterial::computeQpProperties()
{
  // Calculate bnds
  Real sum_eta_i = 0.0;
  for (unsigned int i = 0; i < _neta; ++i)
    sum_eta_i += (*_eta[i])[_qp] * (*_eta[i])[_qp];

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

  // Equilibrium vacancy concentration
  Real cv_eq = 1.0;

  // Calculate the void phase density and potentials
  _rhov[_qp] = _w[_qp] / (_Va * _Va * _kv[_qp]) + cv_eq / _Va;
  _drhovdw[_qp] = 1.0 / (_Va * _Va * _kv[_qp]);

  _omegav[_qp] = -0.5 * _w[_qp] * _w[_qp] / (_Va * _Va * _kv[_qp]) - _w[_qp] * cv_eq / _Va;
  _domegavdw[_qp] = -_rhov[_qp];
  _d2omegavdw2[_qp] = -_drhovdw[_qp];

  // Calculate solid phase density and potential
  Real d3rhosdw3 = 0;
  switch (_solid_energy)
  {
    case 0: // PARABOLIC
    {
      _rhos[_qp] = _w[_qp] / (_Va * _Va * _ks[_qp]) + _cs_eq[_qp] / _Va;
      _drhosdw[_qp] = 1.0 / (_Va * _Va * _ks[_qp]);
      _d2rhosdw2[_qp] = 0.0;
      d3rhosdw3 = 0.0;

      _omegas[_qp] =
          -0.5 * _w[_qp] * _w[_qp] / (_Va * _Va * _ks[_qp]) - _w[_qp] * _cs_eq[_qp] / _Va;
      _domegasdw[_qp] = -_rhos[_qp];
      _d2omegasdw2[_qp] = -_drhosdw[_qp];

      // bodyforce and matreact coefficients for strict mass conservation case
      _hv_c_min[_qp] = _hv[_qp] * 1.0;
      _hs_c_min[_qp] = _hs[_qp] * _cs_eq[_qp];
      _hv_over_kVa[_qp] = _hv[_qp] / (_Va * _kv[_qp]);
      _hs_over_kVa[_qp] = _hs[_qp] / (_Va * _ks[_qp]);

      for (unsigned int i = 0; i < _neta; ++i)
      {
        (*_drhos[i])[_qp] = (*_dcs_eq[i])[_qp] / _Va;
        (*_d2rhosdwdeta[i])[_qp] = 0.0;
        (*_domegasdeta[i])[_qp] = -_w[_qp] * (*_dcs_eq[i])[_qp] / _Va;
        (*_d2omegasdwdeta[i])[_qp] = -(*_dcs_eq[i])[_qp] / _Va;
        for (unsigned int j = i; j < _neta; ++j)
        {
          (*_d2rhos[i][j])[_qp] = (*_d2cs_eq[i][j])[_qp] / _Va;
          (*_d2omegasdetadeta[i][j])[_qp] = -_w[_qp] * (*_d2cs_eq[i][j])[_qp] / _Va;
        }
      }
      break;
    }       // case 0; // PARABOLIC
    case 1: // DILUTE
    {
      Real rho_exp = std::exp(_w[_qp] / _kB / _T[_qp]);
      _rhos[_qp] = _cs_eq[_qp] / _Va * rho_exp;
      _drhosdw[_qp] = _rhos[_qp] / _kB / _T[_qp];
      _d2rhosdw2[_qp] = _drhosdw[_qp] / _kB / _T[_qp];
      d3rhosdw3 = _d2rhosdw2[_qp] / _kB / _T[_qp];

      _omegas[_qp] = _kB * _T[_qp] * (_cs_eq[_qp] / _Va - _rhos[_qp]);
      _domegasdw[_qp] = -_rhos[_qp];
      _d2omegasdw2[_qp] = -_drhosdw[_qp];
      for (unsigned int i = 0; i < _neta; ++i)
      {
        (*_drhos[i])[_qp] = (*_dcs_eq[i])[_qp] * rho_exp / _Va;
        (*_d2rhosdwdeta[i])[_qp] = 0.0;
        (*_domegasdeta[i])[_qp] = _kB * _T[_qp] * (*_dcs_eq[i])[_qp] / _Va * (1.0 - rho_exp);
        (*_d2omegasdwdeta[i])[_qp] = -1.0 / _Va * (*_dcs_eq[i])[_qp] * rho_exp;
        for (unsigned int j = i; j < _neta; ++j)
        {
          (*_d2rhos[i][j])[_qp] = (*_d2cs_eq[i][j])[_qp] * rho_exp / _Va;
          (*_d2omegasdetadeta[i][j])[_qp] =
              _kB * _T[_qp] * (*_d2cs_eq[i][j])[_qp] / _Va * (1.0 - rho_exp);
        }
      }
      break;
    }       // case 1: // DILUTE
    case 2: // IDEAL
    {
      Real Ef = -_kB * _T[_qp] * std::log(_cs_eq[_qp] / (1.0 - _cs_eq[_qp]));
      std::vector<Real> dEf;
      std::vector<std::vector<Real>> d2Ef;
      dEf.resize(_neta);
      d2Ef.resize(_neta);

      Real x = std::exp((_w[_qp] - Ef) / (_kB * _T[_qp]));
      Real x0 = std::exp(-Ef / (_kB * _T[_qp]));
      _rhos[_qp] = x / ((1.0 + x) * _Va);
      Real rhos0 = x0 / ((1.0 + x0) * _Va);
      _drhosdw[_qp] = x / (Utility::pow<2>(1.0 + x) * _Va * _kB * _T[_qp]);
      _d2rhosdw2[_qp] =
          x * (1.0 - x) / (_Va * Utility::pow<2>(_kB * _T[_qp]) * Utility::pow<3>(1.0 + x));
      d3rhosdw3 = x * (1 - 4.0 * x + x * x) /
                  (_Va * Utility::pow<3>(_kB * _T[_qp]) * Utility::pow<4>(1.0 + x));

      _omegas[_qp] = _kB * _T[_qp] / _Va * (std::log(1.0 + x0) - std::log(1.0 + x));
      _domegasdw[_qp] = -_rhos[_qp];
      _d2omegasdw2[_qp] = -_drhosdw[_qp];
      for (unsigned int i = 0; i < _neta; ++i)
      {
        dEf[i] =
            -_kB * _T[_qp] * (*_dcs_eq[i])[_qp] * (1.0 / _cs_eq[_qp] + 1.0 / (1.0 - _cs_eq[_qp]));
        d2Ef[i].resize(_neta);

        (*_drhos[i])[_qp] = -dEf[i] * _drhosdw[_qp];
        (*_d2rhosdwdeta[i])[_qp] = -dEf[i] * _d2rhosdw2[_qp];

        (*_domegasdeta[i])[_qp] = dEf[i] * (_rhos[_qp] - rhos0);
        (*_d2omegasdwdeta[i])[_qp] = dEf[i] * _drhosdw[_qp];

        for (unsigned int j = i; j < _neta; ++j)
        {
          d2Ef[i][j] = -_kB * _T[_qp] *
                       ((*_d2cs_eq[i][j])[_qp] * (1.0 / _cs_eq[_qp] + 1.0 / (1.0 - _cs_eq[_qp])) +
                        (*_dcs_eq[i])[_qp] * (*_dcs_eq[j])[_qp] *
                            (1.0 / ((1.0 - _cs_eq[_qp]) * (1.0 - _cs_eq[_qp])) -
                             1.0 / (_cs_eq[_qp] * _cs_eq[_qp])));

          (*_d2rhos[i][j])[_qp] = -d2Ef[i][j] * _drhosdw[_qp] + dEf[i] * dEf[j] * _d2rhosdw2[_qp];
          (*_d2omegasdetadeta[i][j])[_qp] =
              d2Ef[i][j] * (_rhos[_qp] - rhos0) +
              dEf[i] * dEf[j] / (_Va * _kB * _T[_qp]) *
                  (x / Utility::pow<2>(1.0 + x) - x0 / Utility::pow<2>(1.0 + x0));
        }
      }
      break;
    } // case 2: // IDEAL
  }   // switch (_solid_energy)

  // Calculate the susceptibility
  _chi[_qp] = _hs[_qp] * _drhosdw[_qp] + _hv[_qp] * _drhovdw[_qp];
  _dchidphi[_qp] = _dhs[_qp] * _drhosdw[_qp] + _dhv[_qp] * _drhovdw[_qp];
  _dchidw[_qp] = _hs[_qp] * _d2rhosdw2[_qp];
  _d2chidphi2[_qp] = _d2hs[_qp] * _drhosdw[_qp] + _d2hv[_qp] * _drhovdw[_qp];
  _d2chidw2[_qp] = _hs[_qp] * d3rhosdw3;
  _d2chidphidw[_qp] = _dhs[_qp] * _d2rhosdw2[_qp];

  // thermodynamic parameters
  _mu[_qp] = _mu_gb + (_mu_s - _mu_gb) * f;
  _kappa[_qp] = _kappa_gb + (_kappa_s - _kappa_gb) * f;
  _dmu[_qp] = (_mu_s - _mu_gb) * df;
  _dkappa[_qp] = (_kappa_s - _kappa_gb) * df;
  _d2mu[_qp] = (_mu_s - _mu_gb) * d2f;
  _d2kappa[_qp] = (_kappa_s - _kappa_gb) * d2f;
  _gamma[_qp] = 1.5;
} // void GrandPotentialSinteringMaterial::computeQpProperties()
