//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSXeVacSolidMaterial.h"

registerMooseObject("PhaseFieldApp", KKSXeVacSolidMaterial);

InputParameters
KKSXeVacSolidMaterial::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBase::validParams();
  params.addClassDescription("KKS Solid phase free energy for Xe,Vac in UO2.  Fm(cmg,cmv)");
  params.addRequiredParam<Real>("T", "Temperature in [K]");
  params.addRequiredCoupledVar("cmg", "Gas concentration");
  params.addRequiredCoupledVar("cmv", "Vacancy concentration");
  return params;
}

KKSXeVacSolidMaterial::KKSXeVacSolidMaterial(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters),
    _T(getParam<Real>("T")),
    _Omega(2.53),
    _kB(8.6173324e-5),
    _Efv(3.0),
    _Efg(3.0),
    _cmg(coupledValue("cmg")),
    _cmg_var(coupled("cmg")),
    _cmv(coupledValue("cmv")),
    _cmv_var(coupled("cmv"))
{
}

// Catch fixable singularity at 0
Real
KKSXeVacSolidMaterial::cLogC(Real c)
{
  return c <= 0.0 ? 0.0 : c * std::log(c);
}

// / Fm(cmg,cmv) takes three arguments
unsigned int
KKSXeVacSolidMaterial::expectedNumArgs()
{
  return 2;
}

// Free energy value
Real
KKSXeVacSolidMaterial::computeF()
{
  return 1.0 / _Omega *
         (_kB * _T * (cLogC(_cmv[_qp]) + cLogC(1.0 - _cmv[_qp])) + _Efv * _cmv[_qp] +
          _kB * _T * (cLogC(_cmg[_qp]) + cLogC(1.0 - _cmg[_qp])) + _Efg * _cmg[_qp]);
}

// Derivative of the Free energy
Real
KKSXeVacSolidMaterial::computeDF(unsigned int i_var)
{
  const Real tol = 1e-10;
  Real cmg = _cmg[_qp] < tol ? tol : (_cmg[_qp] > (1.0 - tol) ? (1.0 - tol) : _cmg[_qp]);
  Real cmv = _cmv[_qp] < tol ? tol : (_cmv[_qp] > (1.0 - tol) ? (1.0 - tol) : _cmv[_qp]);

  if (i_var == _cmg_var)
    return 1.0 / _Omega * (_Efg + _kB * _T * (std::log(cmg) - std::log(-cmg + 1.0)));

  if (i_var == _cmv_var)
    return 1.0 / _Omega * (_Efv + _kB * _T * (std::log(cmv) - std::log(-cmv + 1.0)));

  mooseError("Unknown derivative requested");
}

// Derivative of the Free energy
Real
KKSXeVacSolidMaterial::computeD2F(unsigned int i_var, unsigned int j_var)
{
  if (i_var != j_var)
    return 0.0;

  const Real tol = 1e-10;
  Real cmg = _cmg[_qp] < tol ? tol : (_cmg[_qp] > (1.0 - tol) ? (1.0 - tol) : _cmg[_qp]);
  Real cmv = _cmv[_qp] < tol ? tol : (_cmv[_qp] > (1.0 - tol) ? (1.0 - tol) : _cmv[_qp]);

  if (i_var == _cmg_var)
    return 1.0 / _Omega * _kB * _T * (1.0 / (1.0 - cmg) + 1.0 / cmg);

  if (i_var == _cmv_var)
    return 1.0 / _Omega * _kB * _T * (1.0 / (1.0 - cmv) + 1.0 / cmv);

  mooseError("Unknown derivative requested");
}

// note that the second cross derivatives are actually 0.0!

// Third derivative: 1.0/Omega * kB*T*((-c^m_v + 1.0)**(-2) - 1/c^m_v**2)
