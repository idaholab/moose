#include "FlowRegimeBaseMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "FlowModelTwoPhase.h"
#include "Numerics.h"
#include "CHFTable.h"

template <>
InputParameters
validParams<FlowRegimeBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("A", "Cross-section area");
  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("alpha_liquid", "Volume fraction of the liquid phase");
  params.addRequiredCoupledVar("alpha_vapor", "Volume fraction of the vapor phase");
  params.addRequiredCoupledVar("arhoA_liquid", "Coupled arhoA_liquid");
  params.addRequiredCoupledVar("arhoA_vapor", "Coupled arhoA_vapor");
  params.addRequiredCoupledVar("e_liquid", "Coupled liquid phase specific internal energy");
  params.addRequiredCoupledVar("e_vapor", "Coupled vapor phase specific internal energy");
  params.addRequiredCoupledVar("vel_liquid", "Coupled liquid phase velocity");
  params.addRequiredCoupledVar("vel_vapor", "Coupled vapor phase velocity");
  params.addRequiredCoupledVar("rho_liquid", "Coupled liquid phase density");
  params.addRequiredCoupledVar("rho_vapor", "Coupled vapor phase density");
  params.addRequiredCoupledVar("v_liquid", "Coupled liquid phase specific volume");
  params.addRequiredCoupledVar("v_vapor", "Coupled vapor phase specific volume");
  params.addRequiredCoupledVar("D_h", "Hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("mu_liquid",
                                                "Liquid dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_vapor",
                                                "Vapor dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>(
      "cp_liquid", "Liquid constant-pressure specific heat material property");
  params.addRequiredParam<MaterialPropertyName>(
      "cp_vapor", "Vapor constant-pressure specific heat material property");
  params.addRequiredParam<MaterialPropertyName>("k_liquid",
                                                "Liquid thermal conductivity material property");
  params.addRequiredParam<MaterialPropertyName>("k_vapor",
                                                "Vapor thermal conductivity material property");
  params.addRequiredParam<MaterialPropertyName>("p_liquid", "Liquid pressure material property");
  params.addRequiredParam<MaterialPropertyName>("p_vapor", "Vapor pressure material property");
  params.addRequiredParam<MaterialPropertyName>("T_liquid", "Liquid temperature material property");
  params.addRequiredParam<MaterialPropertyName>("T_vapor", "Vapor temperature material property");
  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Surface tension material property");
  params.addRequiredParam<MaterialPropertyName>("h_liquid", "Liquid specific enthalpy");
  params.addRequiredParam<MaterialPropertyName>("h_vapor", "Vapor specific enthalpy");

  params.addRequiredParam<UserObjectName>("fp", "Coupled fluid properties");
  params.addRequiredParam<MooseEnum>(
      "ht_geom", FlowChannel::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  params.addRequiredParam<Real>("alpha_v_min", "vapor volume fraction below which is all liquid");
  params.addRequiredParam<Real>("alpha_v_max", "vapor volume fraction above which is all vapor");
  params.addRequiredParam<Real>("gravity_angle",
                                "Angle between orientation vector and gravity vector, in degrees");
  params.addRequiredParam<bool>("horizontal",
                                "True for horizontal pipes, false for vertical pipes.");
  params.addParam<Real>("PoD", 0, "Pitch-to-diameter ratio (needed for rod bundle).");
  params.addCoupledVar("q_wall", "Wall heat flux");
  params.addRequiredParam<UserObjectName>(
      "chf_table", "The name of the user object name with critical heat flux table");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");
  params.addRequiredParam<bool>(
      "mass_transfer", "Fluid properties support mass transfer and mass transfer is enabled");

  return params;
}

FlowRegimeBaseMaterial::FlowRegimeBaseMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _area(coupledValue("A")),
    _D_h(coupledValue("D_h")),
    _ht_geom(THM::stringToEnum<FlowChannel::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),
    _PoD(getParam<Real>("PoD")),
    _alpha_liquid(coupledValue("alpha_liquid")),
    _dalpha_liquid_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha_liquid", "beta")),
    _alpha_vapor(coupledValue("alpha_vapor")),
    _arhoA_liquid(coupledValue("arhoA_liquid")),
    _arhoA_vapor(coupledValue("arhoA_vapor")),
    _e_liquid(coupledValue("e_liquid")),
    _e_vapor(coupledValue("e_vapor")),
    _velocity_liquid(coupledValue("vel_liquid")),
    _velocity_vapor(coupledValue("vel_vapor")),
    _rho_liquid(coupledValue("rho_liquid")),
    _rho_vapor(coupledValue("rho_vapor")),
    _v_liquid(coupledValue("v_liquid")),
    _v_vapor(coupledValue("v_vapor")),
    _has_q_wall(isParamValid("q_wall")),
    _q_wall(_has_q_wall ? &coupledValue("q_wall") : nullptr),
    _alpha_v_min(getParam<Real>("alpha_v_min")),
    _alpha_v_max(getParam<Real>("alpha_v_max")),
    _gravity_angle(getParam<Real>("gravity_angle")),
    _is_horizontal(getParam<bool>("horizontal")),
    _mu_liquid(getMaterialProperty<Real>("mu_liquid")),
    _mu_vapor(getMaterialProperty<Real>("mu_vapor")),
    _cp_liquid(getMaterialProperty<Real>("cp_liquid")),
    _cp_vapor(getMaterialProperty<Real>("cp_vapor")),
    _k_liquid(getMaterialProperty<Real>("k_liquid")),
    _k_vapor(getMaterialProperty<Real>("k_vapor")),
    _p_liquid(getMaterialProperty<Real>("p_liquid")),
    _p_vapor(getMaterialProperty<Real>("p_vapor")),
    _T_liquid(getMaterialProperty<Real>("T_liquid")),
    _T_vapor(getMaterialProperty<Real>("T_vapor")),
    _surface_tension(getMaterialProperty<Real>("surface_tension")),
    _h_liquid(getMaterialProperty<Real>("h_liquid")),
    _h_vapor(getMaterialProperty<Real>("h_vapor")),
    _kappa_liquid(declareProperty<Real>(FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID)),
    _dkappa_liquid_dbeta(declarePropertyDerivativeTHM<Real>(
        FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID, "beta")),
    _gravity_magnitude(getParam<Real>("gravity_magnitude")),
    _fp(getUserObject<TwoPhaseFluidProperties>("fp")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getVaporName())),
    _chf_table(getUserObject<CHFTable>("chf_table")),
    _mass_transfer(getParam<bool>("mass_transfer"))
{
  if (_ht_geom == FlowChannel::ROD_BUNDLE && _PoD == 0.)
    mooseError(name(), ": When using ROD_BUNDLE geometry, you have to provide the PoD parameter.");
}
