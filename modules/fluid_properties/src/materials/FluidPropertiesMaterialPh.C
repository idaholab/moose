
#include "FluidPropertiesMaterialPh.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesMaterialPh);

InputParameters
FluidPropertiesMaterialPh::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("enthalpy", "Fluid enthalpy (kJ/kg)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Fluid properties using the (pressure, enthalpy) formulation");
  return params;
}

FluidPropertiesMaterialPh::FluidPropertiesMaterialPh(const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _enthalpy(coupledValue("enthalpy")),
    _rho(declareProperty<Real>("density")),
    _T(declareProperty<Real>("temperature")),
    _drho_dp(declareProperty<Real>("drho_dp")),
    _drho_dh(declareProperty<Real>("drho_dh")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialPh::~FluidPropertiesMaterialPh() {}

void
FluidPropertiesMaterialPh::computeQpProperties()
{
  // 定义混合物焓值，汽、液饱和焓值，汽、液饱和密度
  Real h = _enthalpy[_qp];
  Real h_v_sat = _fp.h_v_sat_from_p(_pressure[_qp]);
  Real h_l_sat = _fp.h_l_sat_from_p(_pressure[_qp]);
  Real T_sat = _fp.T_from_p_h(_pressure[_qp], h_l_sat);
  Real rho_v_sat = _fp.rho_from_p_T(_pressure[_qp], (T_sat + 1e-1));
  Real rho_l_sat = _fp.rho_from_p_T(_pressure[_qp], (T_sat - 1e-1));
  // Real drho_v_sat_dh = ;
  // Real drho_l_sat_dh = ;

  // 确定两相状态
  if(h > h_l_sat && h < h_v_sat)
  {
    // 两相
    // 质量含气率
    Real x = (h - h_l_sat) / (h_v_sat - h_l_sat);

    // Bankoff 模型计算含气率
    Real K_Bankoff = 0.71 + 1.45e-8 * _pressure[_qp];
    Real K = K_Bankoff;
    // 空泡份额
    Real alpha = K * rho_l_sat * x / (rho_v_sat * (1 - x) + rho_l_sat * x);
    // 混合物密度
    _rho[_qp] = alpha * rho_v_sat + (1 - alpha) * rho_l_sat;
    // _drho_dp[_qp] = alpha * drho_v_dp + (1 - alpha) * drho_l_dp;
    _T[_qp] = T_sat;
    _drho_dp[_qp] = _fp.drho_dp_from_p_T(_pressure[_qp], _T[_qp]);
    _drho_dh[_qp] = alpha / x * _fp.drho_dh_from_p_T(_pressure[_qp], _T[_qp]) + (1 - alpha) / (1 - x) * _fp.drho_dh_from_p_T(_pressure[_qp], _T[_qp]);
  }
  else if (h < h_l_sat || h > h_v_sat)
  {
    // // 单相，h=h, T(P,h), rho(P,T)
    _T[_qp] = _fp.T_from_p_h(_pressure[_qp], _enthalpy[_qp]);

    _rho[_qp] = _fp.rho_from_p_T(_pressure[_qp], _T[_qp]);
    _drho_dp[_qp] = _fp.drho_dp_from_p_T(_pressure[_qp], _T[_qp]);
    _drho_dh[_qp] = _fp.drho_dh_from_p_T(_pressure[_qp], _T[_qp]);
  }
  else
    mooseError("This shouldn't happen if you have a reasonable value of enthalpy.");

}
