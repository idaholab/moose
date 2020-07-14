#include "PrimitiveVarMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "NS.h"
#include "AuxiliarySystem.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", PrimitiveVarMaterial);

defineADValidParams(
    PrimitiveVarMaterial,
    VarMaterialBase,
    params.addRequiredCoupledVar(nms::pressure, "pressure");
    params.addRequiredCoupledVar(nms::T_fluid, "fluid temperature");
    params.addRequiredCoupledVar(nms::velocity_x, "x-direction velocity");
    params.addCoupledVar(nms::velocity_y, "y-direction velocity");
    params.addCoupledVar(nms::velocity_z, "z-direction velocity");
    params.addClassDescription("Provides access to variables for a primitive variable set "
      "of pressure, temperature, and interstitial velocity"););

PrimitiveVarMaterial::PrimitiveVarMaterial(const InputParameters & params)
  : VarMaterialBase(params),
    _var_T_fluid(adCoupledValue(nms::T_fluid)),
    _var_pressure(adCoupledValue(nms::pressure)),
    _var_vel_x(adCoupledValue(nms::velocity_x)),
    _var_vel_y(isCoupled(nms::velocity_y) ? adCoupledValue(nms::velocity_y) : _ad_zero),
    _var_vel_z(isCoupled(nms::velocity_z) ? adCoupledValue(nms::velocity_z) : _ad_zero),
    _var_grad_T_fluid(adCoupledGradient(nms::T_fluid)),
    _var_grad_pressure(adCoupledGradient(nms::pressure)),
    _var_grad_vel_x(adCoupledGradient(nms::velocity_x)),
    _var_grad_vel_y(isCoupled(nms::velocity_y) ? adCoupledGradient(nms::velocity_y) : _ad_grad_zero),
    _var_grad_vel_z(isCoupled(nms::velocity_z) ? adCoupledGradient(nms::velocity_z) : _ad_grad_zero),
    _var_T_fluid_dot(_is_transient ? adCoupledDot(nms::T_fluid) : _ad_zero),
    _var_pressure_dot(_is_transient ? adCoupledDot(nms::pressure) : _ad_zero),
    _var_vel_x_dot(_is_transient ? adCoupledDot(nms::velocity_x) : _ad_zero),
    _var_vel_y_dot(isCoupled(nms::velocity_y) && _is_transient ? adCoupledDot(nms::velocity_y)
                                                          : _ad_zero),
    _var_vel_z_dot(isCoupled(nms::velocity_z) && _is_transient ? adCoupledDot(nms::velocity_z)
                                                          : _ad_zero),
    _var_grad_grad_T_fluid(adCoupledSecond(nms::T_fluid)),
    _var_grad_grad_vel_x(adCoupledSecond(nms::velocity_x)),
    _var_grad_grad_vel_y(isCoupled(nms::velocity_y) ? adCoupledSecond(nms::velocity_y) : _ad_second_zero),
    _var_grad_grad_vel_z(isCoupled(nms::velocity_z) ? adCoupledSecond(nms::velocity_z) : _ad_second_zero)
{
  warnAuxiliaryVariables();
}

using MetaPhysicL::raw_value;

bool
PrimitiveVarMaterial::coupledAuxiliaryVariables() const
{
  auto & sys = _fe_problem.getAuxiliarySystem();
  return sys.hasVariable(nms::pressure) || sys.hasVariable(nms::T_fluid) ||
      sys.hasVariable(nms::velocity_x) || sys.hasVariable(nms::velocity_y) || sys.hasVariable(nms::velocity_z);
}

void
PrimitiveVarMaterial::setNonlinearProperties()
{
  _T_fluid[_qp] = _var_T_fluid[_qp];
  _pressure[_qp] = _var_pressure[_qp];
  _velocity[_qp] = {_var_vel_x[_qp], _var_vel_y[_qp], _var_vel_z[_qp]};
  _speed[_qp] = computeSpeed();

  _grad_vel_x[_qp] = _var_grad_vel_x[_qp];
  _grad_vel_y[_qp] = _var_grad_vel_y[_qp];
  _grad_vel_z[_qp] = _var_grad_vel_z[_qp];

  _grad_T_fluid[_qp] = _var_grad_T_fluid[_qp];
  _grad_pressure[_qp] = _var_grad_pressure[_qp];

  _dT_dt[_qp] = _var_T_fluid_dot[_qp];

  _grad_grad_T_fluid[_qp] = _var_grad_grad_T_fluid[_qp];
  _grad_grad_vel_x[_qp] = _var_grad_grad_vel_x[_qp];
  _grad_grad_vel_y[_qp] = _var_grad_grad_vel_y[_qp];
  _grad_grad_vel_z[_qp] = _var_grad_grad_vel_z[_qp];
}

void
PrimitiveVarMaterial::computeQpProperties()
{
  setNonlinearProperties();

  Real dummy = 0;

  Real de_dp = 0;
  Real de_dT = 0;
  _fluid.e_from_p_T(raw_value(_pressure[_qp]), raw_value(_T_fluid[_qp]), dummy, de_dp, de_dT);
  _e[_qp] = _fluid.e_from_p_T(_pressure[_qp], _T_fluid[_qp]);

  ADReal drho_dp;
  ADReal drho_dT;
  ADReal rho;
  _fluid.rho_from_p_T(_pressure[_qp], _T_fluid[_qp], rho, drho_dp, drho_dT);

  _rho[_qp] = rho;
  _rhoE[_qp] = _rho[_qp] * (_e[_qp] + 0.5 * (_velocity[_qp] * _velocity[_qp]));
  _momentum[_qp] = _rho[_qp] * _velocity[_qp];
  _v[_qp] = 1.0 / _rho[_qp];

  _grad_rho[_qp] = _grad_pressure[_qp] * drho_dp + _grad_T_fluid[_qp] * drho_dT;

  auto grad_e = _grad_pressure[_qp] * de_dp + _grad_T_fluid[_qp] * de_dT;
  _grad_rhoE[_qp] = _rho[_qp] * (grad_e + _velocity[_qp](0) * _grad_vel_x[_qp] + _velocity[_qp](1) * _grad_vel_y[_qp] + _velocity[_qp](2) * _grad_vel_z[_qp]) +
                    _rhoE[_qp] / _rho[_qp] * _grad_rho[_qp];

  _grad_rho_u[_qp] = _rho[_qp] * _grad_vel_x[_qp] + _velocity[_qp](0) * _grad_rho[_qp];
  _grad_rho_v[_qp] = _rho[_qp] * _grad_vel_y[_qp] + _velocity[_qp](1) * _grad_rho[_qp];
  _grad_rho_w[_qp] = _rho[_qp] * _grad_vel_z[_qp] + _velocity[_qp](2) * _grad_rho[_qp];

  auto de_dt = _var_pressure_dot[_qp] * de_dp + _var_T_fluid_dot[_qp] * de_dT;

  _drho_dt[_qp] = _var_pressure_dot[_qp] * drho_dp + _var_T_fluid_dot[_qp] * drho_dT;
  _drhoE_dt[_qp] =
      _rho[_qp] * de_dt +
      _rho[_qp] * (_velocity[_qp](0) * _var_vel_x_dot[_qp] + _velocity[_qp](1) * _var_vel_y_dot[_qp] + _velocity[_qp](2) * _var_vel_z_dot[_qp]) +
      _rhoE[_qp] / _rho[_qp] * _drho_dt[_qp];
  _drho_u_dt[_qp] = _rho[_qp] * _var_vel_x_dot[_qp] + _velocity[_qp](0) * _drho_dt[_qp];
  _drho_v_dt[_qp] = _rho[_qp] * _var_vel_y_dot[_qp] + _velocity[_qp](1) * _drho_dt[_qp];
  _drho_w_dt[_qp] = _rho[_qp] * _var_vel_z_dot[_qp] + _velocity[_qp](2) * _drho_dt[_qp];

  _enthalpy[_qp] = (_rhoE[_qp] + _pressure[_qp]) / _rho[_qp];
}
