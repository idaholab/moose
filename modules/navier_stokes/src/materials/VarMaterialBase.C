#include "VarMaterialBase.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "AuxiliarySystem.h"
#include "MooseUtils.h"

InputParameters
VarMaterialBase::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addClassDescription(
      "Provides access to nonlinear and aux-like variables independent of formulation");
  return params;
}

VarMaterialBase::VarMaterialBase(const InputParameters & params)
  : Material(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _rho(declareADProperty<Real>(NS::density)),
    _momentum(declareADProperty<RealVectorValue>(NS::momentum)),
    _specific_internal_energy(declareADProperty<Real>(NS::specific_internal_energy)),
    _specific_total_energy(declareADProperty<Real>(NS::specific_total_energy)),
    _total_energy_density(declareADProperty<Real>(NS::total_energy_density)),
    _specific_total_enthalpy(declareADProperty<Real>(NS::specific_total_enthalpy)),
    _total_enthalpy_density(declareADProperty<Real>(NS::total_enthalpy_density)),
    _v(declareADProperty<Real>(NS::v)),
    _T_fluid(declareADProperty<Real>(NS::T_fluid)),
    _pressure(declareADProperty<Real>(NS::pressure)),
    _speed(declareADProperty<Real>(NS::speed)),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _grad_rho(declareADProperty<RealVectorValue>(NS::grad(NS::density))),
    _grad_rho_et(declareADProperty<RealVectorValue>(NS::grad(NS::total_energy_density))),
    _grad_rho_u(declareADProperty<RealVectorValue>(NS::grad(NS::momentum_x))),
    _grad_rho_v(declareADProperty<RealVectorValue>(NS::grad(NS::momentum_y))),
    _grad_rho_w(declareADProperty<RealVectorValue>(NS::grad(NS::momentum_z))),
    _grad_vel_x(declareADProperty<RealVectorValue>(NS::grad(NS::velocity_x))),
    _grad_vel_y(declareADProperty<RealVectorValue>(NS::grad(NS::velocity_y))),
    _grad_vel_z(declareADProperty<RealVectorValue>(NS::grad(NS::velocity_z))),
    _grad_T_fluid(declareADProperty<RealVectorValue>(NS::grad(NS::T_fluid))),
    _grad_pressure(declareADProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _drho_dt(declareADProperty<Real>(NS::time_deriv(NS::density))),
    _drho_et_dt(declareADProperty<Real>(NS::time_deriv(NS::total_energy_density))),
    _drho_u_dt(declareADProperty<Real>(NS::time_deriv(NS::momentum_x))),
    _drho_v_dt(declareADProperty<Real>(NS::time_deriv(NS::momentum_y))),
    _drho_w_dt(declareADProperty<Real>(NS::time_deriv(NS::momentum_z))),
    _dT_dt(declareADProperty<Real>(NS::time_deriv(NS::T_fluid))),
    _grad_grad_T_fluid(declareADProperty<RealTensorValue>(NS::grad(NS::grad(NS::T_fluid)))),
    _grad_grad_vel_x(declareADProperty<RealTensorValue>(NS::grad(NS::grad(NS::velocity_x)))),
    _grad_grad_vel_y(declareADProperty<RealTensorValue>(NS::grad(NS::grad(NS::velocity_y)))),
    _grad_grad_vel_z(declareADProperty<RealTensorValue>(NS::grad(NS::grad(NS::velocity_z))))
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
