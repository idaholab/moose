#include "KKSXeVacSolidMaterial.h"

template<>
InputParameters validParams<KKSXeVacSolidMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("KKS Solid phase free energy for Xe,Vac in UO2.  Fm(cmg,cmv)");
  params.addRequiredParam<Real>("T", "Temperature in [K]");
  return params;
}

KKSXeVacSolidMaterial::KKSXeVacSolidMaterial(const std::string & name,
                                             InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters),
    _T(getParam<Real>("T")),
    _Omega(2.53),
    _kB(8.6173324e-5),
    _Efv(3.0),
    _Efg(3.0)
{
}

// Catch fixable singularity at 0
Real
KKSXeVacSolidMaterial::cLogC(Real c) {
  return c <= 0.0 ? 0.0 : c * std::log(c);
}

// / Fm(cmg,cmv) takes three arguments
unsigned int
KKSXeVacSolidMaterial::expectedNumArgs() { return 2; }

// Free energy value
Real
KKSXeVacSolidMaterial::computeF() {
  // create named aliases for the arguments
  const Real & cmg = (*_args[0])[_qp];
  const Real & cmv = (*_args[1])[_qp];

  return 1.0/_Omega * (
      _kB*_T * (cLogC(cmv) + cLogC(1.0-cmv)) + _Efv * cmv
    + _kB*_T * (cLogC(cmg) + cLogC(1.0-cmg)) + _Efg * cmg
  );
}

// Derivative of the Free energy
Real
KKSXeVacSolidMaterial::computeDF(unsigned int arg) {
  Real cmg = (*_args[0])[_qp];
  Real cmv = (*_args[1])[_qp];

  const Real tol = 1e-10;
  cmg = cmg < tol ? tol : (cmg > (1.0-tol) ? (1.0-tol) : cmg);
  cmv = cmv < tol ? tol : (cmv > (1.0-tol) ? (1.0-tol) : cmv);

  switch (arg)
  {
    case 0: // d/dcmg
      return 1.0/_Omega * (_Efg + _kB*_T * (std::log(cmg) - std::log(-cmg + 1.0)));

    case 1: // d/dcmv
      return 1.0/_Omega * (_Efv + _kB*_T * (std::log(cmv) - std::log(-cmv + 1.0)));
  }

  mooseError("Unknown derivative requested");
}

// Derivative of the Free energy
Real
KKSXeVacSolidMaterial::computeD2F(unsigned int arg1, unsigned int arg2) {
  Real cmg = (*_args[0])[_qp];
  Real cmv = (*_args[1])[_qp];

  if (arg1 != arg2) return 0.0;

  const Real tol = 1e-10;
  cmg = cmg < tol ? tol : (cmg > (1.0-tol) ? (1.0-tol) : cmg);
  cmv = cmv < tol ? tol : (cmv > (1.0-tol) ? (1.0-tol) : cmv);

  switch (arg1)
  {
    case 0: // d/dcmg
      return 1.0/_Omega * _kB*_T * (1.0 / (1.0 - cmg) + 1.0 / cmg);

    case 1: // d/dcmv
      return 1.0/_Omega * _kB*_T * (1.0 / (1.0 - cmv) + 1.0 / cmv);
  }

  mooseError("Unknown derivative requested");
}

// note that the second cross derivatives are actually 0.0!

// Third derivative: 1.0/Omega * kB*T*((-c^m_v + 1.0)**(-2) - 1/c^m_v**2)
