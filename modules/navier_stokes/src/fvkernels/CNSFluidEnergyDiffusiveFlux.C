//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFluidEnergyDiffusiveFlux.h"
#include "NS.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSFluidEnergyDiffusiveFlux);

defineADValidParams(
    CNSFluidEnergyDiffusiveFlux,
    CNSKernel,
    params.addRequiredCoupledVar(nms::porosity, "porosity");
    params.addClassDescription("Diffusive flux $-\\nabla\\cdot(\\kappa_f\\nabla T_f)$ in the "
                               "fluid energy conservation equation."););

CNSFluidEnergyDiffusiveFlux::CNSFluidEnergyDiffusiveFlux(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<CNSKernel>(parameters),
    _eps(coupledValue(nms::porosity)),
    _grad_eps(coupledGradient(nms::porosity)),
    _grad_T_fluid(getADMaterialProperty<RealVectorValue>(nms::grad(nms::T_fluid))),
    _grad_grad_T_fluid(getADMaterialProperty<RealTensorValue>(nms::grad(nms::grad(nms::T_fluid)))),
    _kappa(getADMaterialProperty<Real>(nms::kappa)),
    _dkappa_dp(getMaterialPropertyDerivative<Real>(nms::kappa, nms::pressure)),
    _dkappa_dT(getMaterialPropertyDerivative<Real>(nms::kappa, nms::T_fluid)),
    _grad_pressure(getADMaterialProperty<RealVectorValue>(nms::grad(nms::pressure)))
{
}

ADReal
CNSFluidEnergyDiffusiveFlux::weakResidual()
{
  return _eps[_qp] * _kappa[_qp] * _grad_T_fluid[_qp] * _grad_test[_i][_qp];
}

ADReal
CNSFluidEnergyDiffusiveFlux::strongResidual()
{
  ADRealVectorValue grad_kappa = _dkappa_dT[_qp] * _grad_T_fluid[_qp] +
    _dkappa_dp[_qp] * _grad_pressure[_qp];

  ADReal value = -_eps[_qp] * _kappa[_qp] * _grad_grad_T_fluid[_qp].tr() -
    _grad_T_fluid[_qp] * (_eps[_qp] * grad_kappa + _kappa[_qp] * _grad_eps[_qp]);

  if (_assembly.coordSystem() == Moose::COORD_RZ)
  {
    ADReal r = _q_point[_qp](_rz_coord);
    value += - _eps[_qp] * _kappa[_qp] * _grad_T_fluid[_qp](_rz_coord) / r;
  }

  return value;
}
