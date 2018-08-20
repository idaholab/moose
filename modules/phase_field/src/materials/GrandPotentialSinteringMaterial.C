/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#include "GrandPotentialSinteringMaterial.h"
#include "libmesh/quadrature.h"

registerMooseObject("MarmotApp", GrandPotentialSinteringMaterial);

template <>
InputParameters
validParams<GrandPotentialSinteringMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Includes switching and thermodynamic properties for the grand potential sintering model");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of order parameters that describe solid phase");
  params.addRequiredCoupledVar("chemical_potential", "The name of the chemical potential variable");
  params.addRequiredCoupledVar("void_op", "The name of the void phase order parameter");
  params.addRequiredCoupledVar("Temperature", "Name of the temperature variable with units of K");
  params.addRequiredParam<MaterialPropertyName>(
      "solid_energy_coefficient", "Parabolic solid energy coefficient (energy/volume)");
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
  params.addParam<Real>("vacancy_formation_energy", 2.69, "Vacancy formation energy in eV");
  params.addParam<Real>("GB_vacancy_concentration",
                        0.189,
                        "Extra vacancy concentration on grain boundaries compared to in the bulk");
  params.addParam<Real>("atomic_volume", 0.04092, "Atomic volume of material");
  params.addParam<Real>("vacancy_prefactor",
                        1.0,
                        "Prefactor for Arrhenius function describing the vancancy concentration");
  return params;
}

GrandPotentialSinteringMaterial::GrandPotentialSinteringMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _neta(coupledComponents("etas")),
    _eta(_neta),
    _eta_name(_neta),
    _w(coupledValue("chemical_potential")),
    _w_name(getVar("chemical_potential", 0)->name()),
    _phi(coupledValue("void_op")),
    _phi_name(getVar("void_op", 0)->name()),
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
    _dchi(declarePropertyDerivative<Real>("chi", _phi_name)),
    _d2chi(declarePropertyDerivative<Real>("chi", _phi_name, _phi_name)),
    _rhov(declareProperty<Real>("rhov")),
    _drhovdw(declarePropertyDerivative<Real>("rhov", _w_name)),
    _rhos(declareProperty<Real>("rhos")),
    _drhosdw(declarePropertyDerivative<Real>("rhos", _w_name)),
    _drhos(_neta),
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
    _sigma_s(getParam<Real>("surface_energy")),
    _sigma_gb(getParam<Real>("grainboundary_energy")),
    _int_width(getParam<Real>("int_width")),
    _switch(getParam<Real>("surface_switch_value")),
    _Ef(getParam<Real>("vacancy_formation_energy")),
    _c_gb(getParam<Real>("GB_vacancy_concentration")),
    _Va(getParam<Real>("atomic_volume")),
    _prefactor(getParam<Real>("vacancy_prefactor")),
    _mu_s(6.0 * _sigma_s / _int_width),
    _mu_gb(6.0 * _sigma_gb / _int_width),
    _kappa_s(0.75 * _sigma_s * _int_width),
    _kappa_gb(0.75 * _sigma_gb * _int_width),
    _kB(8.617343e-5) // eV/K
{
  if ((_switch > 1.0) || (_switch < 0.0))
    mooseError("GrandPotentialSinteringMaterial: surface_switch_value should be between 0 and 1");

  for (unsigned int i = 0; i < _neta; ++i)
  {
    _eta[i] = &coupledValue("etas", i);
    _eta_name[i] = getVar("etas", i)->name();
    _drhos[i] = &declarePropertyDerivative<Real>("rhos", _eta_name[i]);
    _d2rhos[i].resize(_neta);
    _domegasdeta[i] = &declarePropertyDerivative<Real>("omegas", _eta_name[i]);
    _d2omegasdwdeta[i] = &declarePropertyDerivative<Real>("omegas", _w_name, _eta_name[i]);
    _d2omegasdetadeta[i].resize(_neta);

    for (unsigned int j = 0; j <= i; ++j)
    {
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

  // Calculate the susceptibility
  _chi[_qp] = (_hs[_qp] / _ks[_qp] + _hv[_qp] / _kv[_qp]) / (_Va * _Va);
  _dchi[_qp] = (_dhs[_qp] / _ks[_qp] + _dhv[_qp] / _kv[_qp]) / (_Va * _Va);
  _d2chi[_qp] = (_d2hs[_qp] / _ks[_qp] + _d2hv[_qp] / _kv[_qp]) / (_Va * _Va);

  // Calculate Densities and potentials
  Real cv_eq = 1.0;
  Real cs_eq = _prefactor * std::exp(-_Ef / (_kB * _T[_qp])) +
               _c_gb * 4.0 * (1.0 - sum_eta_i) * (1.0 - sum_eta_i);

  std::vector<Real> dcs_eq;
  std::vector<std::vector<Real>> d2cs_eq;
  dcs_eq.resize(_neta);
  d2cs_eq.resize(_neta);

  for (unsigned int i = 0; i < _neta; ++i)
  {
    dcs_eq[i] = 16.0 * _c_gb * (1.0 - sum_eta_i) * (*_eta[i])[_qp];
    d2cs_eq[i].resize(_neta);
    d2cs_eq[i][i] = 16.0 * _c_gb * (1.0 - sum_eta_i - 2.0 * (*_eta[i])[_qp] * (*_eta[i])[_qp]);
    for (unsigned int j = i + 1; j < _neta; ++j)
      d2cs_eq[i][j] = -32.0 * _c_gb * (*_eta[i])[_qp] * (*_eta[j])[_qp];
  }

  _rhov[_qp] = _w[_qp] / (_Va * _Va * _kv[_qp]) + cv_eq / _Va;
  _rhos[_qp] = _w[_qp] / (_Va * _Va * _ks[_qp]) + cs_eq / _Va;
  _drhovdw[_qp] = 1.0 / (_Va * _Va * _kv[_qp]);
  _drhosdw[_qp] = 1.0 / (_Va * _Va * _ks[_qp]);
  for (unsigned int i = 0; i < _neta; ++i)
  {
    (*_drhos[i])[_qp] = dcs_eq[i] / _Va;
    for (unsigned int j = i; j < _neta; ++j)
      (*_d2rhos[i][j])[_qp] = d2cs_eq[i][j] / _Va;
  }

  _omegav[_qp] = -0.5 * _w[_qp] * _w[_qp] / (_Va * _Va * _kv[_qp]) - _w[_qp] * cv_eq / _Va;
  _omegas[_qp] = -0.5 * _w[_qp] * _w[_qp] / (_Va * _Va * _ks[_qp]) - _w[_qp] * cs_eq / _Va;

  _domegavdw[_qp] = -_w[_qp] / (_Va * _Va * _kv[_qp]) - cv_eq / _Va;
  _domegasdw[_qp] = -_w[_qp] / (_Va * _Va * _ks[_qp]) - cs_eq / _Va;
  _d2omegavdw2[_qp] = -1.0 / (_Va * _Va * _kv[_qp]);
  _d2omegasdw2[_qp] = -1.0 / (_Va * _Va * _ks[_qp]);
  for (unsigned int i = 0; i < _neta; ++i)
  {
    (*_domegasdeta[i])[_qp] = -_w[_qp] * dcs_eq[i] / _Va;
    (*_d2omegasdwdeta[i])[_qp] = -dcs_eq[i] / _Va;
    for (unsigned int j = i; j < _neta; ++j)
      (*_d2omegasdetadeta[i][j])[_qp] = -_w[_qp] * d2cs_eq[i][j] / _Va;
  }

  // thermodynamic parameters
  _mu[_qp] = _mu_gb + (_mu_s - _mu_gb) * f;
  _kappa[_qp] = _kappa_gb + (_kappa_s - _kappa_gb) * f;
  _dmu[_qp] = (_mu_s - _mu_gb) * df;
  _dkappa[_qp] = (_kappa_s - _kappa_gb) * df;
  _d2mu[_qp] = (_mu_s - _mu_gb) * d2f;
  _d2kappa[_qp] = (_kappa_s - _kappa_gb) * d2f;
  _gamma[_qp] = 1.5;
}
