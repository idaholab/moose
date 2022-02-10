//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidProperties3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", FluidProperties3EqnMaterial);

InputParameters
FluidProperties3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");

  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");

  return params;
}

FluidProperties3EqnMaterial::FluidProperties3EqnMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _area(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rho(declareProperty<Real>("rho")),
    _drho_drhoA(declarePropertyDerivativeTHM<Real>("rho", "rhoA")),

    _v(declareProperty<Real>("v")),
    _dv_drhoA(declarePropertyDerivativeTHM<Real>("v", "rhoA")),

    _vel(declareProperty<Real>("vel")),
    _dvel_drhoA(declarePropertyDerivativeTHM<Real>("vel", "rhoA")),
    _dvel_drhouA(declarePropertyDerivativeTHM<Real>("vel", "rhouA")),

    _e(declareProperty<Real>("e")),
    _de_drhoA(declarePropertyDerivativeTHM<Real>("e", "rhoA")),
    _de_drhouA(declarePropertyDerivativeTHM<Real>("e", "rhouA")),
    _de_drhoEA(declarePropertyDerivativeTHM<Real>("e", "rhoEA")),

    _p(declareProperty<Real>("p")),
    _dp_drhoA(declarePropertyDerivativeTHM<Real>("p", "rhoA")),
    _dp_drhouA(declarePropertyDerivativeTHM<Real>("p", "rhouA")),
    _dp_drhoEA(declarePropertyDerivativeTHM<Real>("p", "rhoEA")),

    _T(declareProperty<Real>("T")),
    _dT_drhoA(declarePropertyDerivativeTHM<Real>("T", "rhoA")),
    _dT_drhouA(declarePropertyDerivativeTHM<Real>("T", "rhouA")),
    _dT_drhoEA(declarePropertyDerivativeTHM<Real>("T", "rhoEA")),

    _h(declareProperty<Real>("h")),
    _dh_drhoA(declarePropertyDerivativeTHM<Real>("h", "rhoA")),
    _dh_drhouA(declarePropertyDerivativeTHM<Real>("h", "rhouA")),
    _dh_drhoEA(declarePropertyDerivativeTHM<Real>("h", "rhoEA")),

    _H(declareProperty<Real>("H")),
    _dH_drhoA(declarePropertyDerivativeTHM<Real>("H", "rhoA")),
    _dH_drhouA(declarePropertyDerivativeTHM<Real>("H", "rhouA")),
    _dH_drhoEA(declarePropertyDerivativeTHM<Real>("H", "rhoEA")),

    _c(declareProperty<Real>("c")),

    _cp(declareProperty<Real>("cp")),

    _cv(declareProperty<Real>("cv")),

    _k(declareProperty<Real>("k")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

void
FluidProperties3EqnMaterial::computeQpProperties()
{
  _rho[_qp] = _rhoA[_qp] / _area[_qp];
  _drho_drhoA[_qp] = 1.0 / _area[_qp];

  _v[_qp] = 1.0 / _rho[_qp];
  _dv_drhoA[_qp] = THM::dv_darhoA(_area[_qp], _rhoA[_qp]);

  THM::vel_from_arhoA_arhouA(
      _rhoA[_qp], _rhouA[_qp], _vel[_qp], _dvel_drhoA[_qp], _dvel_drhouA[_qp]);

  _e[_qp] = (_rhoEA[_qp] - 0.5 * _rhouA[_qp] * _rhouA[_qp] / _rhoA[_qp]) / _rhoA[_qp];
  _de_drhoA[_qp] = THM::de_darhoA(_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp]);
  _de_drhouA[_qp] = THM::de_darhouA(_rhoA[_qp], _rhouA[_qp]);
  _de_drhoEA[_qp] = THM::de_darhoEA(_rhoA[_qp]);

  _p[_qp] = _fp.p_from_v_e(_v[_qp], _e[_qp]);
  Real p, dp_dv, dp_de;
  _fp.p_from_v_e(_v[_qp], _e[_qp], p, dp_dv, dp_de);

  _T[_qp] = _fp.T_from_v_e(_v[_qp], _e[_qp]);
  Real T, dT_dv, dT_de;
  _fp.T_from_v_e(_v[_qp], _e[_qp], T, dT_dv, dT_de);

  _dp_drhoA[_qp] = dp_dv * _dv_drhoA[_qp] + dp_de * _de_drhoA[_qp];
  _dp_drhouA[_qp] = dp_de * _de_drhouA[_qp];

  _dT_drhoA[_qp] = dT_dv * _dv_drhoA[_qp] + dT_de * _de_drhoA[_qp];
  _dT_drhouA[_qp] = dT_de * _de_drhouA[_qp];

  _dp_drhoEA[_qp] = dp_de * _de_drhoEA[_qp];
  _dT_drhoEA[_qp] = dT_de * _de_drhoEA[_qp];

  _h[_qp] = _e[_qp] + _p[_qp] / _rho[_qp];
  const Real dh_de = 1;
  const Real dh_dp = 1.0 / _rho[_qp];
  const Real dh_drho = -_p[_qp] / _rho[_qp] / _rho[_qp];
  _dh_drhoA[_qp] = dh_de * _de_drhoA[_qp] + dh_dp * _dp_drhoA[_qp] + dh_drho * _drho_drhoA[_qp];
  _dh_drhouA[_qp] = dh_de * _de_drhouA[_qp] + dh_dp * _dp_drhouA[_qp];
  _dh_drhoEA[_qp] = dh_de * _de_drhoEA[_qp] + dh_dp * _dp_drhoEA[_qp];

  _H[_qp] = _h[_qp] + 0.5 * _vel[_qp] * _vel[_qp];
  _dH_drhoA[_qp] = _dh_drhoA[_qp] + _vel[_qp] * _dvel_drhoA[_qp];
  _dH_drhouA[_qp] = _dh_drhouA[_qp] + _vel[_qp] * _dvel_drhouA[_qp];
  _dH_drhoEA[_qp] = _dh_drhoEA[_qp];

  _c[_qp] = _fp.c_from_v_e(_v[_qp], _e[_qp]);
  _cp[_qp] = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  _cv[_qp] = _fp.cv_from_v_e(_v[_qp], _e[_qp]);
  _k[_qp] = _fp.k_from_v_e(_v[_qp], _e[_qp]);
}
