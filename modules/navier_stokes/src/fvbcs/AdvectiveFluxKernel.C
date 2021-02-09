//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxKernel.h"
#include "NS.h"

namespace nms = NS;

defineADValidParams(AdvectiveFluxKernel,
                    CNSKernel,
                    params.addRequiredCoupledVar(nms::porosity, "porosity");
                    params.addClassDescription("Base kernel for defining the conservative form of an advective flux"););

AdvectiveFluxKernel::AdvectiveFluxKernel(const InputParameters & parameters)
  : CNSKernel(parameters),
    _eps(coupledValue(nms::porosity)),
    _grad_eps(coupledGradient(nms::porosity)),
    _rho(getADMaterialProperty<Real>(nms::density)),
    _velocity(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _grad_rho(getADMaterialProperty<RealVectorValue>(nms::grad(nms::density))),
    _grad_vel_x(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_x))),
    _grad_vel_y(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_y))),
    _grad_vel_z(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_z))),
    _grad_rho_u(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_x))),
    _grad_rho_v(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_y))),
    _grad_rho_w(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_z)))
{
}

ADReal
AdvectiveFluxKernel::weakResidual()
{
  return -_eps[_qp] * _rho[_qp] * _velocity[_qp] * advectedField() * _grad_test[_i][_qp];
}

ADReal
AdvectiveFluxKernel::velocityDivergence() const
{
  return _grad_vel_x[_qp](0) + _grad_vel_y[_qp](1) + _grad_vel_z[_qp](2);
}
