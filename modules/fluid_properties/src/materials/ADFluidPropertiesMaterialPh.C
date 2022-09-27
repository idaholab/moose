
#include "ADFluidPropertiesMaterialPh.h"
#include "Function.h"
#include "Assembly.h"
#include "FEProblemBase.h"

registerMooseObject("FluidPropertiesApp", ADFluidPropertiesMaterialPh);

InputParameters
ADFluidPropertiesMaterialPh::validParams()
{
  InputParameters params = Material::validParams();
  // 求解变量 u，p，h
  params.addRequiredCoupledVar("velocity", "The velocity");
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("enthalpy", "Fluid enthalpy (kJ/kg)");

  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Fluid properties using the (pressure, enthalpy) formulation");
  return params;
}

ADFluidPropertiesMaterialPh::ADFluidPropertiesMaterialPh(const InputParameters & parameters)
  : Material(parameters),
    _pressure(adCoupledValue("pressure")),
    _grad_p(adCoupledGradient("pressure")),

    _enthalpy(adCoupledValue("enthalpy")),
    _grad_enthalpy(adCoupledGradient("enthalpy")),

    _velocity(adCoupledVectorValue("velocity")),
    _grad_velocity(adCoupledVectorGradient("velocity")),

    _rho(declareADProperty<Real>("density")),
    _T(declareADProperty<Real>("temperature")),

    _drho_dp(declareADProperty<Real>("drho_dp")),
    _drho_dh(declareADProperty<Real>("drho_dh")),

    _mu(declareADProperty<Real>("viscocity")),

    // _advective_strong_residual(declareADProperty<RealVectorValue>("advective_strong_residual")),
    // _enthalpy_advective_strong_residual(declareADProperty<RealVectorValue>("enthalpy_advective_strong_residual")),

    // _velocity_dot(nullptr),
    // _enthalpy_dot(nullptr),
    //
    // _td_strong_residual(declareADProperty<RealVectorValue>("td_strong_residual")),
    // _enthalpy_td_strong_residual(declareADProperty<Real>("enthalpy_td_strong_residual")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

ADFluidPropertiesMaterialPh::~ADFluidPropertiesMaterialPh() {}

void
ADFluidPropertiesMaterialPh::computeQpProperties()
{
  // 对饱和线处的函数进行修改+-
  // Water97FluidProperties::h_l_sat_from_p()
  // Water97FluidProperties::h_v_sat_from_p()
  // ADFluidPropertiesMaterialPh::computeQpProperties()::rho_l_sat
  // ADFluidPropertiesMaterialPh::computeQpProperties()::rho_v_sat
  // 定义混合物焓值，汽、液饱和焓值，汽、液饱和密度
  ADReal h = MetaPhysicL::raw_value(_enthalpy[_qp]);

  ADReal h_v_sat = _fp.h_v_sat_from_p(MetaPhysicL::raw_value(_pressure[_qp]));
  ADReal h_l_sat = _fp.h_l_sat_from_p(MetaPhysicL::raw_value(_pressure[_qp]));
  ADReal T_sat = _fp.vaporTemperature(MetaPhysicL::raw_value(_pressure[_qp]));//T_from_p_h(MetaPhysicL::raw_value(_pressure[_qp]), h_l_sat);
  ADReal rho_v_sat = _fp.rho_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), (T_sat));// + 1e-1
  ADReal rho_l_sat = _fp.rho_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), (T_sat));// - 1e-1
  // Real drho_v_sat_dh = ;
  // Real drho_l_sat_dh = ;

  // 确定两相状态：计算质量含气率、空泡份额、混合物密度、温度、密度对压力和焓值的偏导
  if(h > h_l_sat && h < h_v_sat)
  {
    // 两相
    // 质量含气率
    ADReal x = (h - h_l_sat) / (h_v_sat - h_l_sat);

    // Bankoff 模型计算含气率
    ADReal K_Bankoff = 0.71 + 1.45e-8 * MetaPhysicL::raw_value(_pressure[_qp]);
    ADReal K = K_Bankoff;
    // 空泡份额
    ADReal alpha = K * rho_l_sat * x / (rho_v_sat * (1 - x) + rho_l_sat * x);
    // 混合物密度
    _rho[_qp] = alpha * rho_v_sat + (1 - alpha) * rho_l_sat;
    // _drho_dp[_qp] = alpha * drho_v_dp + (1 - alpha) * drho_l_dp;
    _T[_qp] = T_sat;
    _drho_dp[_qp] = _fp.drho_dp_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
    _drho_dh[_qp] = alpha / x * _fp.drho_dh_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp])
                    + (1 - alpha) / (1 - x) * _fp.drho_dh_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
  }
  else if (h < h_l_sat || h > h_v_sat)
  {
    // // 单相，h=h, T(P,h), rho(P,T)
    _T[_qp] = _fp.T_from_p_h(MetaPhysicL::raw_value(_pressure[_qp]), MetaPhysicL::raw_value(_enthalpy[_qp]));

    _mu[_qp] = _fp.mu_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
    _rho[_qp] = _fp.rho_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
    _drho_dp[_qp] = _fp.drho_dp_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
    _drho_dh[_qp] = _fp.drho_dh_from_p_T(MetaPhysicL::raw_value(_pressure[_qp]), _T[_qp]);
  }
  else
    mooseError("This shouldn't happen if you have a reasonable value of enthalpy.");

  // _advective_strong_residual[_qp] = _rho[_qp] * _grad_velocity[_qp] * _velocity[_qp];
  // _velocity_dot = &adCoupledVectorDot("velocity");
  // _td_strong_residual[_qp] = _rho[_qp] * (*_velocity_dot)[_qp];

  // _enthalpy_advective_strong_residual[_qp] = _rho[_qp] * _velocity[_qp] * _grad_enthalpy[_qp];
  // _enthalpy_dot = &adCoupledDot("enthalpy");
  // _enthalpy_td_strong_residual[_qp] = _rho[_qp] * (*_enthalpy_dot)[_qp];
}
