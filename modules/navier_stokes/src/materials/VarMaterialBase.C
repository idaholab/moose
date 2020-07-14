#include "VarMaterialBase.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "AuxiliarySystem.h"
#include "MooseUtils.h"

namespace nms = NS;

defineADValidParams(
    VarMaterialBase,
    ADMaterial,
    params.addRequiredParam<UserObjectName>(nms::fluid, "fluid userobject");
    params.addClassDescription(
        "Provides access to nonlinear and aux-like variables independent of formulation"););

VarMaterialBase::VarMaterialBase(const InputParameters & params)
  : ADMaterial(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(nms::fluid)),
    _rho(declareADProperty<Real>(nms::density)),
    _rhoE(declareADProperty<Real>(nms::rho_et)),
    _momentum(declareADProperty<RealVectorValue>(nms::momentum)),
    _enthalpy(declareADProperty<Real>(nms::enthalpy)),
    _e(declareADProperty<Real>(nms::e)),
    _v(declareADProperty<Real>(nms::v)),
    _T_fluid(declareADProperty<Real>(nms::T_fluid)),
    _pressure(declareADProperty<Real>(nms::pressure)),
    _speed(declareADProperty<Real>(nms::speed)),
    _velocity(declareADProperty<RealVectorValue>(nms::velocity)),
    _grad_rho(declareADProperty<RealVectorValue>(nms::grad(nms::density))),
    _grad_rhoE(declareADProperty<RealVectorValue>(nms::grad(nms::rho_et))),
    _grad_rho_u(declareADProperty<RealVectorValue>(nms::grad(nms::momentum_x))),
    _grad_rho_v(declareADProperty<RealVectorValue>(nms::grad(nms::momentum_y))),
    _grad_rho_w(declareADProperty<RealVectorValue>(nms::grad(nms::momentum_z))),
    _grad_vel_x(declareADProperty<RealVectorValue>(nms::grad(nms::velocity_x))),
    _grad_vel_y(declareADProperty<RealVectorValue>(nms::grad(nms::velocity_y))),
    _grad_vel_z(declareADProperty<RealVectorValue>(nms::grad(nms::velocity_z))),
    _grad_T_fluid(declareADProperty<RealVectorValue>(nms::grad(nms::T_fluid))),
    _grad_pressure(declareADProperty<RealVectorValue>(nms::grad(nms::pressure))),
    _drho_dt(declareADProperty<Real>(nms::time_deriv(nms::density))),
    _drhoE_dt(declareADProperty<Real>(nms::time_deriv(nms::rho_et))),
    _drho_u_dt(declareADProperty<Real>(nms::time_deriv(nms::momentum_x))),
    _drho_v_dt(declareADProperty<Real>(nms::time_deriv(nms::momentum_y))),
    _drho_w_dt(declareADProperty<Real>(nms::time_deriv(nms::momentum_z))),
    _dT_dt(declareADProperty<Real>(nms::time_deriv(nms::T_fluid))),
    _grad_grad_T_fluid(declareADProperty<RealTensorValue>(nms::grad(nms::grad(nms::T_fluid)))),
    _grad_grad_vel_x(declareADProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_x)))),
    _grad_grad_vel_y(declareADProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_y)))),
    _grad_grad_vel_z(declareADProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_z))))
{
}

void
VarMaterialBase::warnAuxiliaryVariables() const
{
  if (coupledAuxiliaryVariables())
    mooseDoOnce(mooseWarning(this->name() + " parameters contain auxiliary variables: "
                                            "jacobians may be incorrect."));
}

ADReal
VarMaterialBase::computeSpeed() const
{
  // if the velocity is zero, then the norm function call fails because AD tries to calculate the
  // derivatives which causes a divide by zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx.
  // So add a bit of noise to avoid this failure mode.
  if ((MooseUtils::absoluteFuzzyEqual(_velocity[_qp](0), 0)) &&
      (MooseUtils::absoluteFuzzyEqual(_velocity[_qp](1), 0)) &&
      (MooseUtils::absoluteFuzzyEqual(_velocity[_qp](2), 0)))
    return 1e-42;

  return _velocity[_qp].norm();
}
