//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBAnisotropyMisorientation.h"

registerMooseObject("PhaseFieldApp", GBAnisotropyMisorientation);

InputParameters
GBAnisotropyMisorientation::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Compute of material parameters for multi-order paramater phase field models based on misorientation");
  params.addCoupledVar("T", 300.0, "Temperature in Kelvin");
  params.addRequiredParam<Real>("wGB", "Diffuse GB width in nm");
  params.addParam<Real>("length_scale", 1.0e-9, "Length scale in m, where default is nm");
  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns");
  params.addParam<Real>("molar_volume_value",
                        7.11e-6,
                        "molar volume of material in m^3/mol, by default it's the value of copper");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

GBAnisotropyMisorientation::GBAnisotropyMisorientation(const InputParameters & parameters)
  : Material(parameters), 
    _mesh_dimension(_mesh.dimension()),
    _wGB(getParam<Real>("wGB")),
    _length_scale(getParam<Real>("length_scale")),
    _time_scale(getParam<Real>("time_scale")),
    _M_V(getParam<Real>("molar_volume_value")),
    _T(coupledValue("T")),
    _sigma_gb(declareProperty<Real>("sigma")),
    _mob_gb(declareProperty<Real>("M_GB")),
    _kappa_gb(declareProperty<Real>("kappa_op")),
    _gamma_gb(declareProperty<Real>("gamma_asymm")), // gamma
    _L_gb(declareProperty<Real>("L")),
    _mu_gb(declareProperty<Real>("mu")),
    _molar_volume(declareProperty<Real>("molar_volume")),
    _entropy_diff(declareProperty<Real>("entropy_diff")),
    _act_wGB(declareProperty<Real>("act_wGB")),
    _kb(8.617343e-5),      // Boltzmann constant in eV/K
    _JtoeV(6.24150974e18), // Joule to eV conversion
    _mu_qp(0.0),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _grad_vals(coupledGradients("v")),
    _delta_theta(getMaterialProperty<Real>("delta_theta"))
{

}

void
GBAnisotropyMisorientation::computeQpProperties()
{
  Real sigma_init; //
  Real g2 = 0.0; 
  Real f_interf = 0.0; // f_{0, \text { interf }}(\gamma)}
  Real a_0 = 0.75; // a_init(gamma_init) or a_k
  Real a_star = 0.0; // a*(kappa*, gamma*)
  Real kappa_star = 0.0; 
  Real gamma_star = 0.0;
  Real y = 0.0; // 1/gamma
  Real yyy = 0.0; // 1/gamma^3


  const Real & delta_theta_HGB = 15;
  // Grain boundary mobility according to the sigmoidal law 
  const Real & GBmobi0_HGB = 2.5e-6 * std::exp(- 0.23 / (_kb * 450)); // the grain boudary mobility of a high angle grain boundary
  const Real B = 5;
  const Real n = 4;

  // Grain boundary energy according to the Read-Shockley law;
  const Real & GBEnergy_HGB = 0.708; // the grain boudary energy of a high angle grain boundary  

  _mob = GBmobi0_HGB; // GBmobi0_HGB;
  _sigma = GBEnergy_HGB; //GBEnergy_HGB
  
  if (_delta_theta[_qp] > 0)
  {
    _mob = GBmobi0_HGB * ((1- std::exp(-B * std::pow(_delta_theta[_qp] / delta_theta_HGB, n))) * 4 + 1 );  // 
    if (_delta_theta[_qp] < 15)
      _sigma = GBEnergy_HGB * ( _delta_theta[_qp] / delta_theta_HGB * (1 - std::log(_delta_theta[_qp] / delta_theta_HGB)) * 4 + 1);
    else
      _sigma = GBEnergy_HGB * (15.0 / delta_theta_HGB * (1 - std::log(15.0 / delta_theta_HGB)) * 4 + 1);
  }    

      // Convert units of mobility and energy
  _sigma *= _JtoeV * (_length_scale * _length_scale); // eV/nm^2

  _mob *= _time_scale / (_JtoeV * (_length_scale * _length_scale * _length_scale * _length_scale)); // Convert to nm^4/(eV*ns);                                    

  // 设置初始晶界能和局部自由能密度函数的前置因子
  _mu_qp = 6.0 * _sigma / _wGB; // 3/4 * 0.125 * sigma / l, model coefficient

  // 对于晶粒m-晶粒n的晶界
  a_star = a_0; // 0.75
  a_0 = 0.0; // a_0 = sqrt*(f_{0, interf}(gamma)) / g(gamma)

  while (std::abs(a_0 - a_star) > 1.0e-9) // whie (a_0 != a*)
  {
    a_0 = a_star;
    kappa_star = a_0 * _wGB * _sigma; // (eq-36b)
    g2 = _sigma * _sigma / (kappa_star * _mu_qp); // (eq-12)
    y = -5.288 * g2 * g2 * g2 * g2 - 0.09364 * g2 * g2 * g2 + 9.965 * g2 * g2 - 8.183 * g2 +
        2.007; // g^{-1} ??
    gamma_star = 1 / y; // gamma* = g^{-1}
    yyy = y * y * y;
    f_interf = 0.05676 * yyy * yyy - 0.2924 * yyy * y * y + 0.6367 * yyy * y - 0.7749 * yyy +
                0.6107 * y * y - 0.4324 * y + 0.2792; // f_interf(gamma*)
    a_star = std::sqrt(f_interf / g2);
  }

   _kappa = kappa_star; // upper triangle stores the discrete set of kappa values
  _gamma = gamma_star; // lower triangle stores the discrete set of gamma values

  _a = a_star; // upper triangle stores "a" data. a*
  _g2 = g2;     // lower triangle stores "g2" data. (eq-12)

  _sigma_gb[_qp] = _sigma;
  _mob_gb[_qp] = _mob;
  _kappa_gb[_qp] = _kappa;
  _gamma_gb[_qp] = _gamma;
  _L_gb[_qp] = _mob * std::exp(-_Q / (_kb * _T[_qp])) * _mu_qp * _g2 / _sigma;
  _mu_gb[_qp] = _mu_qp;

  _molar_volume[_qp] =
      _M_V / (_length_scale * _length_scale * _length_scale); // m^3/mol converted to ls^3/mol
  _entropy_diff[_qp] = 9.5 * _JtoeV;                          // J/(K mol) converted to eV(K mol)
  _act_wGB[_qp] = 0.5e-9 / _length_scale;                     // 0.5 nm
}