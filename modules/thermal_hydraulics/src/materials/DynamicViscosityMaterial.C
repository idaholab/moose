//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicViscosityMaterial.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", DynamicViscosityMaterial);

InputParameters
DynamicViscosityMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar("beta", "Remapped volume fraction of liquid");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*vel*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity property");
  params.addRequiredParam<MaterialPropertyName>("v", "Specific volume property");
  params.addRequiredParam<MaterialPropertyName>("e", "Specific internal energy property");

  params.addRequiredParam<UserObjectName>("fp_1phase", "Single-phase fluid properties");

  params.addClassDescription("Computes dynamic viscosity");

  return params;
}

DynamicViscosityMaterial::DynamicViscosityMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    _mu_name(getParam<MaterialPropertyName>("mu")),
    _mu(declareProperty<Real>(_mu_name)),
    _dmu_dbeta(isParamValid("beta") ? &declarePropertyDerivativeTHM<Real>(_mu_name, "beta")
                                    : nullptr),
    _dmu_darhoA(declarePropertyDerivativeTHM<Real>(_mu_name, "arhoA")),
    _dmu_darhouA(declarePropertyDerivativeTHM<Real>(_mu_name, "arhouA")),
    _dmu_darhoEA(declarePropertyDerivativeTHM<Real>(_mu_name, "arhoEA")),

    _v(getMaterialProperty<Real>("v")),
    _dv_dbeta(isParamValid("beta") ? &getMaterialPropertyDerivativeTHM<Real>("v", "beta")
                                   : nullptr),
    _dv_darhoA(getMaterialPropertyDerivativeTHM<Real>("v", "arhoA")),

    _e(getMaterialProperty<Real>("e")),
    _de_darhoA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoA")),
    _de_darhouA(getMaterialPropertyDerivativeTHM<Real>("e", "arhouA")),
    _de_darhoEA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoEA")),

    _fp_1phase(getUserObject<SinglePhaseFluidProperties>("fp_1phase"))
{
}

void
DynamicViscosityMaterial::computeQpProperties()
{
  Real dmu_dv, dmu_de;
  _fp_1phase.mu_from_v_e(_v[_qp], _e[_qp], _mu[_qp], dmu_dv, dmu_de);

  if (isParamValid("beta"))
    (*_dmu_dbeta)[_qp] = dmu_dv * (*_dv_dbeta)[_qp];
  _dmu_darhoA[_qp] = dmu_dv * _dv_darhoA[_qp] + dmu_de * _de_darhoA[_qp];
  _dmu_darhouA[_qp] = dmu_de * _de_darhouA[_qp];
  _dmu_darhoEA[_qp] = dmu_de * _de_darhoEA[_qp];
}
