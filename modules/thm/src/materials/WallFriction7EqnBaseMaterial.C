#include "WallFriction7EqnBaseMaterial.h"

template <>
InputParameters
validParams<WallFriction7EqnBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("beta", "Volume fraction equation variable: beta");
  params.addRequiredCoupledVar("arhoA_liquid", "Liquid mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_liquid", "Liquid momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_liquid", "Liquid energy equation variable: alpha*rho*E*A");
  params.addRequiredCoupledVar("arhoA_vapor", "Vapor mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_vapor", "Vapor momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_vapor", "Vapor energy equation variable: alpha*rho*E*A");
  params.addRequiredCoupledVar("alpha_liquid", "Volume fraction of the liquid phase");
  params.addRequiredCoupledVar("alpha_vapor", "Volume fraction of the vapor phase");
  params.addRequiredCoupledVar("rho_liquid", "Liquid phase density");
  params.addRequiredCoupledVar("rho_vapor", "Vapor phase density");
  params.addRequiredCoupledVar("vel_liquid", "Liquid phase velocity");
  params.addRequiredCoupledVar("vel_vapor", "Vapor phase velocity");
  params.addRequiredCoupledVar("T_liquid", "Liquid phase temperature");
  params.addRequiredCoupledVar("D_h", "hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("mu_liquid",
                                                "Liquid dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_vapor",
                                                "Vapor dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Surface tension material property");
  params.addRequiredParam<MaterialPropertyName>("is_post_CHF", "Is-post-CHF material property");
  params.addRequiredParam<MaterialPropertyName>("wall_drag_flow_regime",
                                                "Wall drag flow regime material property");
  params.addRequiredParam<MaterialPropertyName>("wall_heat_transfer_flow_regime",
                                                "Wall heat transfer flow regime material property");

  params.addParam<Real>("roughness", 0.0, "Surface roughness");
  params.addRequiredParam<bool>("horizontal", "Is pipe horizontal");
  params.addRequiredParam<MooseEnum>(
      "ht_geom", FlowChannel::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  return params;
}

WallFriction7EqnBaseMaterial::WallFriction7EqnBaseMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _is_horizontal(getParam<bool>("horizontal")),
    _ht_geom(THM::stringToEnum<FlowChannel::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),

    _f_D_liquid(declareProperty<Real>("f_D_liquid")),
    _df_D_liquid_dbeta(declarePropertyDerivativeTHM<Real>("f_D_liquid", "beta")),
    _df_D_liquid_drhoAL(declarePropertyDerivativeTHM<Real>("f_D_liquid", "arhoA_liquid")),
    _df_D_liquid_drhouAL(declarePropertyDerivativeTHM<Real>("f_D_liquid", "arhouA_liquid")),
    _df_D_liquid_drhoEAL(declarePropertyDerivativeTHM<Real>("f_D_liquid", "arhoEA_liquid")),

    _f_D_vapor(declareProperty<Real>("f_D_vapor")),
    _df_D_vapor_dbeta(declarePropertyDerivativeTHM<Real>("f_D_vapor", "beta")),
    _df_D_vapor_drhoAV(declarePropertyDerivativeTHM<Real>("f_D_vapor", "arhoA_vapor")),
    _df_D_vapor_drhouAV(declarePropertyDerivativeTHM<Real>("f_D_vapor", "arhouA_vapor")),
    _df_D_vapor_drhoEAV(declarePropertyDerivativeTHM<Real>("f_D_vapor", "arhoEA_vapor")),

    _mu_l(getMaterialProperty<Real>("mu_liquid")),
    _mu_v(getMaterialProperty<Real>("mu_vapor")),
    _surface_tension(getMaterialProperty<Real>("surface_tension")),

    _alpha_liquid(coupledValue("alpha_liquid")),
    _alpha_vapor(coupledValue("alpha_vapor")),
    _rho_l(coupledValue("rho_liquid")),
    _rho_v(coupledValue("rho_vapor")),
    _T_l(coupledValue("T_liquid")),
    _v_l(coupledValue("vel_liquid")),
    _v_v(coupledValue("vel_vapor")),
    _D_h(coupledValue("D_h")),
    _roughness(getParam<Real>("roughness")),
    _is_post_CHF(getMaterialProperty<bool>("is_post_CHF")),
    _wdrag_flow_regime(getMaterialProperty<std::vector<Real>>("wall_drag_flow_regime")),
    _wht_flow_regime(getMaterialProperty<std::vector<Real>>("wall_heat_transfer_flow_regime"))
{
}
