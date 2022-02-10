//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFluidProperties3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADFluidProperties3EqnMaterial);

InputParameters
ADFluidProperties3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");

  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");

  return params;
}

ADFluidProperties3EqnMaterial::ADFluidProperties3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    _area(adCoupledValue("A")),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _rho(declareADProperty<Real>("rho")),

    _v(declareADProperty<Real>("v")),

    _vel(declareADProperty<Real>("vel")),

    _e(declareADProperty<Real>("e")),

    _p(declareADProperty<Real>("p")),

    _T(declareADProperty<Real>("T")),

    _h(declareADProperty<Real>("h")),

    _H(declareADProperty<Real>("H")),

    _c(declareADProperty<Real>("c")),

    _cp(declareADProperty<Real>("cp")),

    _cv(declareADProperty<Real>("cv")),

    _k(declareADProperty<Real>("k")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

void
ADFluidProperties3EqnMaterial::computeQpProperties()
{
  _rho[_qp] = _rhoA[_qp] / _area[_qp];

  _v[_qp] = 1.0 / _rho[_qp];

  _vel[_qp] = _rhouA[_qp] / _rhoA[_qp];

  _e[_qp] = (_rhoEA[_qp] - 0.5 * _rhouA[_qp] * _rhouA[_qp] / _rhoA[_qp]) / _rhoA[_qp];

  _p[_qp] = _fp.p_from_v_e(_v[_qp], _e[_qp]);

  _T[_qp] = _fp.T_from_v_e(_v[_qp], _e[_qp]);

  _h[_qp] = _e[_qp] + _p[_qp] / _rho[_qp];

  _H[_qp] = _h[_qp] + 0.5 * _vel[_qp] * _vel[_qp];

  _c[_qp] = _fp.c_from_v_e(_v[_qp], _e[_qp]);
  _cp[_qp] = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  _cv[_qp] = _fp.cv_from_v_e(_v[_qp], _e[_qp]);
  _k[_qp] = _fp.k_from_v_e(_v[_qp], _e[_qp]);
}
