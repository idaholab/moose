//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectrochemicalDefectMaterial.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", ElectrochemicalDefectMaterial);

InputParameters
ElectrochemicalDefectMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Calculates density, susceptibility, and derivatives for a defect species in the grand "
      "potential sintering model coupled with electrochemistry");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of order parameters that describe solid phase");
  params.addRequiredCoupledVar("chemical_potential", "The name of the chemical potential variable");
  params.addRequiredCoupledVar("void_op", "The name of the void phase order parameter");
  params.addRequiredCoupledVar("Temperature", "Name of the temperature variable with units of K");
  params.addRequiredCoupledVar("electric_potential",
                               "Name of the electric potential variable with units of V");
  params.addParam<MaterialPropertyName>(
      "void_density_name",
      "nv",
      "Name of material property to be created for defect number density in the void phase.");
  params.addParam<MaterialPropertyName>(
      "solid_density_name",
      "ns",
      "Name of material property to be created for defect number density in the solid phase.");
  params.addParam<MaterialPropertyName>(
      "chi_name", "chi", "Name of material property to be created defect susceptibility.");
  params.addParam<MaterialPropertyName>("solid_energy_coefficient",
                                        1.0,
                                        "Parabolic solid energy coefficient (energy*volume) for "
                                        "defect species. Only used for parabolic energy.");
  params.addRequiredParam<MaterialPropertyName>(
      "void_energy_coefficient",
      "Parabolic void energy coefficient (energy*volume) for defect species.");
  params.addRequiredParam<MaterialPropertyName>(
      "min_vacancy_concentration_solid",
      "Name of material that determines the minimum in energy wrt defect concentration in the "
      "solid phase");
  params.addRequiredParam<Real>("min_vacancy_concentration_void",
                                "Minimum in energy wrt defect concentration in the void phase");
  MooseEnum solid_energy_model("PARABOLIC DILUTE IDEAL", "PARABOLIC");
  params.addParam<MooseEnum>("solid_energy_model",
                             solid_energy_model,
                             "Type of energy function to use for the solid phase.");
  params.addRequiredParam<int>("defect_charge", "Effective charge of defect species");
  params.addRequiredParam<Real>("solid_relative_permittivity",
                                "Solid phase relative permittivity (dimensionless)");
  params.addParam<Real>("voltage_scale", 1, "Voltage scale (default is for voltage in V)");
  return params;
}

ElectrochemicalDefectMaterial::ElectrochemicalDefectMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _neta(coupledComponents("etas")),
    _eta(_neta),
    _eta_name(_neta),
    _w(coupledValue("chemical_potential")),
    _w_name(coupledName("chemical_potential", 0)),
    _phi(coupledValue("void_op")),
    _phi_name(coupledName("void_op", 0)),
    _ns_min_name(getParam<MaterialPropertyName>("min_vacancy_concentration_solid")),
    _ns_min(getMaterialProperty<Real>(_ns_min_name)),
    _dns_min(_neta),
    _d2ns_min(_neta),
    _temp(coupledValue("Temperature")),
    _v(coupledValue("electric_potential")),
    _grad_V(coupledGradient("electric_potential")),
    _kv(getMaterialProperty<Real>("void_energy_coefficient")),
    _ks(getMaterialProperty<Real>("solid_energy_coefficient")),
    _hv(getMaterialPropertyByName<Real>("hv")),
    _dhv(getMaterialPropertyDerivativeByName<Real>("hv", _phi_name)),
    _d2hv(getMaterialPropertyDerivativeByName<Real>("hv", _phi_name, _phi_name)),
    _hs(getMaterialPropertyByName<Real>("hs")),
    _dhs(getMaterialPropertyDerivativeByName<Real>("hs", _phi_name)),
    _d2hs(getMaterialPropertyDerivativeByName<Real>("hs", _phi_name, _phi_name)),
    _chi_name(getParam<MaterialPropertyName>("chi_name")),
    _chi(declareProperty<Real>(_chi_name)),
    _dchidphi(declarePropertyDerivative<Real>(_chi_name, _phi_name)),
    _dchidw(declarePropertyDerivative<Real>(_chi_name, _w_name)),
    _d2chidphi2(declarePropertyDerivative<Real>(_chi_name, _phi_name, _phi_name)),
    _d2chidw2(declarePropertyDerivative<Real>(_chi_name, _w_name, _w_name)),
    _d2chidphidw(declarePropertyDerivative<Real>(_chi_name, _phi_name, _w_name)),
    _nv_name(getParam<MaterialPropertyName>("void_density_name")),
    _nv(declareProperty<Real>(_nv_name)),
    _dnvdw(declarePropertyDerivative<Real>(_nv_name, _w_name)),
    _ns_name(getParam<MaterialPropertyName>("solid_density_name")),
    _ns(declareProperty<Real>(_ns_name)),
    _dnsdw(declarePropertyDerivative<Real>(_ns_name, _w_name)),
    _d2nsdw2(declarePropertyDerivative<Real>(_ns_name, _w_name, _w_name)),
    _dns(_neta),
    _d2nsdwdeta(_neta),
    _d2ns(_neta),
    _solid_energy(getParam<MooseEnum>("solid_energy_model")),
    _kB(8.617343e-5), // eV/K
    _e(1.0),          // to put energy units in eV
    _z(getParam<int>("defect_charge")),
    _v_scale(getParam<Real>("voltage_scale")),
    _eps_r(getParam<Real>("solid_relative_permittivity")),
    _nv_min(getParam<Real>("min_vacancy_concentration_void"))
{

  for (unsigned int i = 0; i < _neta; ++i)
  {
    _eta[i] = &coupledValue("etas", i);
    _eta_name[i] = coupledName("etas", i);
    _dns_min[i] = &getMaterialPropertyDerivativeByName<Real>(_ns_min_name, _eta_name[i]);
    _d2ns_min[i].resize(_neta);
    _dns[i] = &declarePropertyDerivative<Real>(_ns_name, _eta_name[i]);
    _d2ns[i].resize(_neta);
    _d2nsdwdeta[i] = &declarePropertyDerivative<Real>(_ns_name, _w_name, _eta_name[i]);

    for (unsigned int j = 0; j <= i; ++j)
    {
      _d2ns_min[j][i] =
          &getMaterialPropertyDerivativeByName<Real>(_ns_min_name, _eta_name[j], _eta_name[i]);
      _d2ns[j][i] = &declarePropertyDerivative<Real>(_ns_name, _eta_name[j], _eta_name[i]);
    }
  }
}

void
ElectrochemicalDefectMaterial::computeQpProperties()
{
  // Calculate the void phase density and derivatives
  _nv[_qp] = (_w[_qp] - _z * _e * _v[_qp] * _v_scale) / _kv[_qp] + _nv_min;
  _dnvdw[_qp] = 1.0 / _kv[_qp]; // susceptibility

  // Calculate solid phase density and derivatives
  Real d3nsdw3 = 0.0;
  switch (_solid_energy)
  {
    case 0: // PARABOLIC
    {
      _ns[_qp] = (_w[_qp] - _z * _e * _v[_qp] * _v_scale) / _ks[_qp] + _ns_min[_qp];
      _dnsdw[_qp] = 1.0 / _ks[_qp];
      _d2nsdw2[_qp] = 0.0;
      d3nsdw3 = 0.0;

      for (unsigned int i = 0; i < _neta; ++i)
      {
        (*_dns[i])[_qp] = (*_dns_min[i])[_qp];
        (*_d2nsdwdeta[i])[_qp] = 0.0;
        for (unsigned int j = i; j < _neta; ++j)
        {
          (*_d2ns[i][j])[_qp] = (*_d2ns_min[i][j])[_qp];
        }
      }
      break;
    }       // case 0; // PARABOLIC
    case 1: // DILUTE
    {
      Real n_exp = std::exp((_w[_qp] - _z * _e * _v[_qp] * _v_scale) / _kB / _temp[_qp]);
      _ns[_qp] = _ns_min[_qp] * n_exp;
      _dnsdw[_qp] = _ns[_qp] / _kB / _temp[_qp];
      _d2nsdw2[_qp] = _dnsdw[_qp] / _kB / _temp[_qp];
      d3nsdw3 = _d2nsdw2[_qp] / _kB / _temp[_qp];

      for (unsigned int i = 0; i < _neta; ++i)
      {
        (*_dns[i])[_qp] = (*_dns_min[i])[_qp] * n_exp;
        (*_d2nsdwdeta[i])[_qp] = (*_dns_min[i])[_qp] * n_exp / _kB / _temp[_qp];
        for (unsigned int j = i; j < _neta; ++j)
        {
          (*_d2ns[i][j])[_qp] = (*_d2ns_min[i][j])[_qp] * n_exp;
        }
      }
      break;
    }       // case 1: // DILUTE
    case 2: // IDEAL
    {
      mooseError("Ideal solution in solid is not yet supported in ElectrochemicalDefectMaterial");
      break;
    } // case 2: // IDEAL
  }   // switch (_solid_energy)

  // Calculate the susceptibilities
  _chi[_qp] = _hs[_qp] * _dnsdw[_qp] + _hv[_qp] * _dnvdw[_qp];
  _dchidphi[_qp] = _dhs[_qp] * _dnsdw[_qp] + _dhv[_qp] * _dnvdw[_qp];
  _dchidw[_qp] = _hs[_qp] * _d2nsdw2[_qp];
  _d2chidphi2[_qp] = _d2hs[_qp] * _dnsdw[_qp] + _d2hv[_qp] * _dnvdw[_qp];
  _d2chidw2[_qp] = _hs[_qp] * d3nsdw3;
  _d2chidphidw[_qp] = _dhs[_qp] * _d2nsdw2[_qp];
} // void ElectrochemicalDefectMaterial::computeQpProperties()
