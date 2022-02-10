//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReynoldsNumberMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "MathUtils.h"

registerMooseObject("ThermalHydraulicsApp", ReynoldsNumberMaterial);

InputParameters
ReynoldsNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar("beta", "Remapped volume fraction of liquid");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*vel*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("Re", "Reynolds number property name");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density of the phase");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity of the phase");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity of the phase");

  params.addClassDescription("Computes Reynolds number as a material property");

  return params;
}

ReynoldsNumberMaterial::ReynoldsNumberMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    _Re_name(getParam<MaterialPropertyName>("Re")),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("rho", "beta")
                                  : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _D_h(getMaterialProperty<Real>("D_h")),

    _mu(getMaterialProperty<Real>("mu")),
    _dmu_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivative<Real>("mu", "beta") : nullptr),
    _dmu_darhoA(getMaterialPropertyDerivative<Real>("mu", "arhoA")),
    _dmu_darhouA(getMaterialPropertyDerivative<Real>("mu", "arhouA")),
    _dmu_darhoEA(getMaterialPropertyDerivative<Real>("mu", "arhoEA")),

    _Re(declareProperty<Real>(_Re_name)),
    _dRe_dbeta(isCoupled("beta") ? &declarePropertyDerivativeTHM<Real>(_Re_name, "beta") : nullptr),
    _dRe_darhoA(declarePropertyDerivativeTHM<Real>(_Re_name, "arhoA")),
    _dRe_darhouA(declarePropertyDerivativeTHM<Real>(_Re_name, "arhouA")),
    _dRe_darhoEA(declarePropertyDerivativeTHM<Real>(_Re_name, "arhoEA"))
{
}

void
ReynoldsNumberMaterial::computeQpProperties()
{
  _Re[_qp] = THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);

  const Real dRe_drho = std::fabs(_vel[_qp]) * _D_h[_qp] / _mu[_qp];
  const Real dRe_dvel = MathUtils::sign(_vel[_qp]) * _rho[_qp] * _D_h[_qp] / _mu[_qp];
  const Real dRe_dmu = -_rho[_qp] * std::fabs(_vel[_qp]) * _D_h[_qp] / std::pow(_mu[_qp], 2);

  if (isCoupled("beta"))
    (*_dRe_dbeta)[_qp] = dRe_drho * (*_drho_dbeta)[_qp] + dRe_dmu * (*_dmu_dbeta)[_qp];
  _dRe_darhoA[_qp] =
      dRe_drho * _drho_darhoA[_qp] + dRe_dvel * _dvel_darhoA[_qp] + dRe_dmu * _dmu_darhoA[_qp];
  _dRe_darhouA[_qp] = dRe_dvel * _dvel_darhouA[_qp] + dRe_dmu * _dmu_darhouA[_qp];
  _dRe_darhoEA[_qp] = dRe_dmu * _dmu_darhoEA[_qp];
}
