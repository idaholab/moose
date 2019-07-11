#include "WallMassTransferSimpleMaterial.h"
#include "FlowModel.h"
#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", WallMassTransferSimpleMaterial);

template <>
InputParameters
validParams<WallMassTransferSimpleMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA_liquid", "Liquid mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_liquid", "Liquid momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_liquid", "Liquid energy equation variable: alpha*rho*E*A");
  params.addRequiredCoupledVar("arhoA_vapor", "Vapor mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_vapor", "Vapor momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_vapor", "Vapor energy equation variable: alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>(
      "Hw_liquid", "Convective heat transfer coefficient of liquid phase");
  params.addRequiredParam<MaterialPropertyName>("heat_flux_partitioning_liquid",
                                                "Heat flux partitioning coefficient of liquid");
  params.addRequiredParam<MaterialPropertyName>("T_liquid", "Liquid temperature material property");
  params.addRequiredParam<MaterialPropertyName>("p_liquid", "Liquid pressure material property");
  params.addRequiredParam<MaterialPropertyName>("p_int_bar",
                                                "Average interfacial pressure material property");
  params.addRequiredParam<MaterialPropertyName>("wall_boiling_fraction",
                                                "Name of wall boiling fraction property");

  params.addRequiredParam<UserObjectName>("fp", "The name of the fluid properties user object.");

  return params;
}

WallMassTransferSimpleMaterial::WallMassTransferSimpleMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _T_wall(getMaterialProperty<Real>("T_wall")),
    _Hw_liquid(getMaterialProperty<Real>("Hw_liquid")),
    _kappa_liquid(getMaterialProperty<Real>("heat_flux_partitioning_liquid")),
    _dkappa_liquid_dbeta(
        getMaterialPropertyDerivativeTHM<Real>("heat_flux_partitioning_liquid", "beta")),

    _tpfp(getUserObject<TwoPhaseFluidProperties>("fp")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_tpfp.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_tpfp.getVaporName())),

    _Gamma_wall(declareProperty<Real>("Gamma_wall")),
    _d_Gamma_wall_dbeta(declarePropertyDerivativeTHM<Real>("Gamma_wall", "beta")),
    _d_Gamma_wall_darhoAL(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhoA_liquid")),
    _d_Gamma_wall_darhouAL(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhouA_liquid")),
    _d_Gamma_wall_darhoEAL(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhoEA_liquid")),
    _d_Gamma_wall_darhoAV(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhoA_vapor")),
    _d_Gamma_wall_darhouAV(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhouA_vapor")),
    _d_Gamma_wall_darhoEAV(declarePropertyDerivativeTHM<Real>("Gamma_wall", "arhoEA_vapor")),

    _T_liquid(getMaterialProperty<Real>("T_liquid")),
    _dTL_dbeta(getMaterialPropertyDerivativeTHM<Real>("T_liquid", "beta")),
    _dTL_darhoAL(getMaterialPropertyDerivativeTHM<Real>("T_liquid", "arhoA_liquid")),
    _dTL_darhouAL(getMaterialPropertyDerivativeTHM<Real>("T_liquid", "arhouA_liquid")),
    _dTL_darhoEAL(getMaterialPropertyDerivativeTHM<Real>("T_liquid", "arhoEA_liquid")),

    _pIbar(getMaterialProperty<Real>("p_int_bar")),
    _dpIbar_dbeta(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "beta")),
    _dpIbar_darhoAL(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhoA_liquid")),
    _dpIbar_darhouAL(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhouA_liquid")),
    _dpIbar_darhoEAL(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhoEA_liquid")),
    _dpIbar_darhoAV(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhoA_vapor")),
    _dpIbar_darhouAV(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhouA_vapor")),
    _dpIbar_darhoEAV(getMaterialPropertyDerivativeTHM<Real>("p_int_bar", "arhoEA_vapor")),

    _p_liquid(getMaterialProperty<Real>("p_liquid")),
    _dpL_dbeta(getMaterialPropertyDerivativeTHM<Real>("p_liquid", "beta")),
    _dpL_darhoAL(getMaterialPropertyDerivativeTHM<Real>("p_liquid", "arhoA_liquid")),
    _dpL_darhouAL(getMaterialPropertyDerivativeTHM<Real>("p_liquid", "arhouA_liquid")),
    _dpL_darhoEAL(getMaterialPropertyDerivativeTHM<Real>("p_liquid", "arhoEA_liquid")),

    _f_boil(getMaterialProperty<Real>("wall_boiling_fraction")),
    _df_boil_dbeta(getMaterialPropertyDerivativeTHM<Real>("wall_boiling_fraction", "beta")),
    _df_boil_darhoA_liquid(
        getMaterialPropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhoA_liquid")),
    _df_boil_darhouA_liquid(
        getMaterialPropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhouA_liquid")),
    _df_boil_darhoEA_liquid(
        getMaterialPropertyDerivativeTHM<Real>("wall_boiling_fraction", "arhoEA_liquid"))
{
}

void
WallMassTransferSimpleMaterial::computeQpProperties()
{
  const Real Tsat_at_pIbar = _tpfp.T_sat(_pIbar[_qp]);
  const Real dTsat_dpIbar = _tpfp.dT_sat_dp(_pIbar[_qp]);

  Real h_liquid, dh_liquid_dpL, dh_liquid_dTL;
  _fp_liquid.h_from_p_T(_p_liquid[_qp], _T_liquid[_qp], h_liquid, dh_liquid_dpL, dh_liquid_dTL);

  Real hIV, dhIV_dpIbar, dhIV_dTsat_at_pIbar;
  _fp_vapor.h_from_p_T(_pIbar[_qp], Tsat_at_pIbar, hIV, dhIV_dpIbar, dhIV_dTsat_at_pIbar);

  Real q_wall_liquid = _kappa_liquid[_qp] * _Hw_liquid[_qp] * (_T_wall[_qp] - _T_liquid[_qp]);
  Real h = hIV - h_liquid;
  _Gamma_wall[_qp] = _f_boil[_qp] * q_wall_liquid / h;

  // derivatives of q_wall_liquid
  Real dq_wall_liquid_dbeta =
      (_dkappa_liquid_dbeta[_qp] * _Hw_liquid[_qp] * (_T_wall[_qp] - _T_liquid[_qp]) -
       _kappa_liquid[_qp] * _Hw_liquid[_qp] * _dTL_dbeta[_qp]);
  Real dq_wall_liquid_darhoAL = (-_kappa_liquid[_qp] * _Hw_liquid[_qp] * _dTL_darhoAL[_qp]);
  Real dq_wall_liquid_darhouAL = (-_kappa_liquid[_qp] * _Hw_liquid[_qp] * _dTL_darhouAL[_qp]);
  Real dq_wall_liquid_darhoEAL = (-_kappa_liquid[_qp] * _Hw_liquid[_qp] * _dTL_darhoEAL[_qp]);
  Real dq_wall_liquid_darhoAV = 0;
  Real dq_wall_liquid_darhouAV = 0;
  Real dq_wall_liquid_darhoEAV = 0;

  // derivatives of h
  Real dh_dbeta = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_dbeta[_qp] -
                  dh_liquid_dTL * _dTL_dbeta[_qp];
  Real dh_darhoAL = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhoAL[_qp] -
                    dh_liquid_dTL * _dTL_darhoAL[_qp];
  Real dh_darhouAL = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhouAL[_qp] -
                     dh_liquid_dTL * _dTL_darhouAL[_qp];
  Real dh_darhoEAL = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhoEAL[_qp] -
                     dh_liquid_dTL * _dTL_darhoEAL[_qp];
  Real dh_darhoAV = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhoAV[_qp];
  Real dh_darhouAV = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhouAV[_qp];
  Real dh_darhoEAV = (dhIV_dpIbar + dhIV_dTsat_at_pIbar * dTsat_dpIbar) * _dpIbar_darhoEAV[_qp];

  _d_Gamma_wall_dbeta[_qp] =
      (_df_boil_dbeta[_qp] * q_wall_liquid / h - _f_boil[_qp] * q_wall_liquid * dh_dbeta / h / h +
       _f_boil[_qp] * dq_wall_liquid_dbeta / h);
  _d_Gamma_wall_darhoAL[_qp] = (_df_boil_darhoA_liquid[_qp] * q_wall_liquid / h -
                                _f_boil[_qp] * q_wall_liquid * dh_darhoAL / h / h +
                                _f_boil[_qp] * dq_wall_liquid_darhoAL / h);
  _d_Gamma_wall_darhouAL[_qp] = (_df_boil_darhouA_liquid[_qp] * q_wall_liquid / h -
                                 _f_boil[_qp] * q_wall_liquid * dh_darhouAL / h / h +
                                 _f_boil[_qp] * dq_wall_liquid_darhouAL / h);
  _d_Gamma_wall_darhoEAL[_qp] = (_df_boil_darhoEA_liquid[_qp] * q_wall_liquid / h -
                                 _f_boil[_qp] * q_wall_liquid * dh_darhoEAL / h / h +
                                 _f_boil[_qp] * dq_wall_liquid_darhoEAL / h);
  _d_Gamma_wall_darhoAV[_qp] = (-_f_boil[_qp] * q_wall_liquid * dh_darhoAV / h / h +
                                _f_boil[_qp] * dq_wall_liquid_darhoAV / h);
  _d_Gamma_wall_darhouAV[_qp] = (-_f_boil[_qp] * q_wall_liquid * dh_darhouAV / h / h +
                                 _f_boil[_qp] * dq_wall_liquid_darhouAV / h);
  _d_Gamma_wall_darhoEAV[_qp] = (-_f_boil[_qp] * q_wall_liquid * dh_darhoEAV / h / h +
                                 _f_boil[_qp] * dq_wall_liquid_darhoEAV / h);
}
