/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FluidPropertiesDerivativeTestMaterial.h"
#include "SinglePhaseFluidPropertiesPT.h"

template<>
InputParameters validParams<FluidPropertiesDerivativeTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "Fluid temperature (K)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<Real>("eps", 1e-8, "Finite differencing epsilon. Default is 1e-8");
  params.addClassDescription("Material to test the derivatives provided in a SinglePhaseFluidPropertiesPT object");
  return params;
}

FluidPropertiesDerivativeTestMaterial::FluidPropertiesDerivativeTestMaterial(const InputParameters & parameters) :
    Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),

    _drho_dp(declareProperty<Real>("drho_dp")),
    _drho_dT(declareProperty<Real>("drho_dT")),
    _de_dp(declareProperty<Real>("de_dp")),
    _de_dT(declareProperty<Real>("de_dT")),
    _dh_dp(declareProperty<Real>("dh_dp")),
    _dh_dT(declareProperty<Real>("dh_dT")),
    _dmu_drho(declareProperty<Real>("dmu_drho")),
    _dmu_dT(declareProperty<Real>("dmu_dT")),
    _drho_dp_fd(declareProperty<Real>("drho_dp_fd")),
    _drho_dT_fd(declareProperty<Real>("drho_dT_fd")),
    _de_dp_fd(declareProperty<Real>("de_dp_fd")),
    _de_dT_fd(declareProperty<Real>("de_dT_fd")),
    _dh_dp_fd(declareProperty<Real>("dh_dp_fd")),
    _dh_dT_fd(declareProperty<Real>("dh_dT_fd")),
    _dmu_drho_fd(declareProperty<Real>("dmu_drho_fd")),
    _dmu_dT_fd(declareProperty<Real>("dmu_dT_fd")),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp")),
    _eps(getParam<Real>("eps"))
{
  _console << name() << " is a material for testing purposes only";
}

FluidPropertiesDerivativeTestMaterial::~FluidPropertiesDerivativeTestMaterial()
{
}

void
FluidPropertiesDerivativeTestMaterial::computeQpProperties()
{
  Real p = _pressure[_qp];
  Real T = _temperature[_qp];

  // Calculate analytical derivatives
  Real rho, drho_dp, drho_dT, e, de_dp, de_dT;
  Real h, dh_dp, dh_dT, mu, dmu_drho, dmu_dT;
  _fp.rho_e_dpT(p, T, rho, drho_dp, drho_dT, e, de_dp, de_dT);
  _fp.h_dpT(p, T, h, dh_dp, dh_dT);
  _fp.mu_drhoT(rho, T, mu, dmu_drho, dmu_dT);

  _drho_dp[_qp] = drho_dp;
  _drho_dT[_qp] = drho_dT;
  _de_dp[_qp] = de_dp;
  _de_dT[_qp] = de_dT;
  _dh_dp[_qp] = dh_dp;
  _dh_dT[_qp] = dh_dT;
  _dmu_drho[_qp] = dmu_drho;
  _dmu_dT[_qp] = dmu_dT;

  // Calculate finite difference derivatives
  Real peps = p * _eps;
  Real Teps = T * _eps;
  Real rhoeps = rho * _eps;

  _drho_dp_fd[_qp] = (_fp.rho(p + peps, T) - _fp.rho(p, T)) / peps;
  _drho_dT_fd[_qp] = (_fp.rho(p, T + Teps) - _fp.rho(p, T)) / Teps;
  _de_dp_fd[_qp] = (_fp.e(p + peps, T) - _fp.e(p, T)) / peps;
  _de_dT_fd[_qp] = (_fp.e(p, T + Teps) - _fp.e(p, T)) / Teps;
  _dh_dp_fd[_qp] = (_fp.h(p + peps, T) - _fp.h(p, T)) / peps;
  _dh_dT_fd[_qp] = (_fp.h(p, T + Teps) - _fp.h(p, T)) / Teps;
  _dmu_drho_fd[_qp] = (_fp.mu(rho + rhoeps, T) - _fp.mu(rho, T)) / rhoeps;
  _dmu_dT_fd[_qp] = (_fp.mu(rho, T + Teps) - _fp.mu(rho, T)) / Teps;
}
