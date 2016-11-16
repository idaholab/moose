/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GBAnisotropyBase.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<GBAnisotropyBase>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("T", 300.0, "Temperature in Kelvin");
  params.addParam<Real>("length_scale", 1.0e-9, "Length scale in m, where default is nm");
  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns");
  params.addParam<Real>("molar_volume_value", 7.11e-6, "molar volume of material in m^3/mol, by default it's the value of copper");
  params.addParam<Real>("delta_sigma", 0.1, "factor determining inclination dependence of GB energy");
  params.addParam<Real>("delta_mob", 0.1, "factor determining inclination dependence of GB mobility");
  params.addRequiredParam<FileName>("Anisotropic_GB_file_name", "Name of the file containing: 1)GB mobility prefactor; 2) GB migration activation energy; 3)GB energy");
  params.addRequiredParam<bool>("inclination_anisotropy", "The GB anisotropy ininclination would be considered if true");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

GBAnisotropyBase::GBAnisotropyBase(const InputParameters & parameters) :
    Material(parameters),
    _mesh_dimension(_mesh.dimension()),
    _length_scale(getParam<Real>("length_scale")),
    _time_scale(getParam<Real>("time_scale")),
    _M_V(getParam<Real>("molar_volume_value")),
    _delta_sigma(getParam<Real>("delta_sigma")),
    _delta_mob(getParam<Real>("delta_mob")),
    _Anisotropic_GB_file_name(getParam<FileName>("Anisotropic_GB_file_name")),
    _inclination_anisotropy(getParam<bool>("inclination_anisotropy")),
    _T(coupledValue("T")),
    _kappa(declareProperty<Real>("kappa_op")),
    _gamma(declareProperty<Real>("gamma_asymm")),
    _L(declareProperty<Real>("L")),
    _mu(declareProperty<Real>("mu")),
    _molar_volume(declareProperty<Real>("molar_volume")),
    _entropy_diff(declareProperty<Real>("entropy_diff")),
    _act_wGB(declareProperty<Real>("act_wGB")),
    _tgrad_corr_mult(declareProperty<Real>("tgrad_corr_mult")),
    _kb(8.617343e-5), // Boltzmann constant in eV/K
    _JtoeV(6.24150974e18), // Joule to eV conversion
    _mu_qp(0.0),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _grad_vals(_op_num)
{
  // reshape vectors
  _sigma.resize(_op_num);
  _mob.resize(_op_num);
  _Q.resize(_op_num);
  _kappa_gamma.resize(_op_num);
  _a_g2.resize(_op_num);

  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Initialize variables
    _vals[op] = &coupledValue("v", op);
    _grad_vals[op] = &coupledGradient("v", op);

    _sigma[op].resize(_op_num);
    _mob[op].resize(_op_num);
    _Q[op].resize(_op_num);
    _kappa_gamma[op].resize(_op_num);
    _a_g2[op].resize(_op_num);
  }

  // Read in data from "Anisotropic_GB_file_name"
  std::ifstream inFile(_Anisotropic_GB_file_name.c_str());

  if (!inFile)
    mooseError("Can't open GB anisotropy input file");

  for (unsigned int i = 0; i < 2; ++i)
    inFile.ignore(255, '\n'); // ignore line

  Real data;
  for (unsigned int i = 0; i < 3 * _op_num; ++i)
  {
    std::vector<Real> row; // create an empty row of double values
    for (unsigned int j = 0; j < _op_num; ++j)
    {
      inFile >> data;
      row.push_back(data);
    }

    if (i < _op_num)
      _sigma[i] = row; // unit: J/m^2

    else if (i < 2 * _op_num)
      _mob[i-_op_num] = row; // unit: m^4/(J*s)

    else
      _Q[i - 2 * _op_num] = row; // unit: eV
  }

  inFile.close();
}

void
GBAnisotropyBase::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _molar_volume[_qp] = _M_V / (_length_scale*_length_scale*_length_scale); // m^3/mol converted to ls^3/mol
    _entropy_diff[_qp] = 9.5 * _JtoeV; // J/(K mol) converted to eV(K mol)
    _act_wGB[_qp] = 0.5e-9 / _length_scale; // 0.5 nm
    _tgrad_corr_mult[_qp] = _mu[_qp] * 9.0/8.0;
  }
}
