#include "WallHeatTransferCoefficient7EqnBaseMaterial.h"
#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"
#include "LiquidFluidPropertiesInterface.h"
#include "CHFTable.h"

template <>
InputParameters
validParams<WallHeatTransferCoefficient7EqnBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MooseEnum>(
      "ht_geom", PipeBase::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  params.addParam<Real>("PoD", 0, "Pitch-to-diameter ratio (needed for rod bundle).");
  params.addRequiredParam<Real>("alpha_v_min", "vapor volume fraction below which is all liquid");
  params.addRequiredParam<Real>("alpha_v_max", "vapor volume fraction above which is all vapor");
  params.addRequiredCoupledVar("arhoA_liquid", "Conserved density of liquid");
  params.addRequiredCoupledVar("arhoA_vapor", "Conserved density of vapor");
  params.addRequiredCoupledVar("A", "Cross-section area");
  params.addRequiredCoupledVar("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("alpha_liquid", "Volume fraction of liquid");
  params.addRequiredParam<MaterialPropertyName>("alpha_vapor", "Volume fraction of vapor");
  params.addRequiredParam<MaterialPropertyName>("rho_liquid", "Density of the liquid");
  params.addRequiredParam<MaterialPropertyName>("rho_vapor", "Density of the vapor");
  params.addRequiredParam<MaterialPropertyName>("p_liquid", "Pressure of the liquid phase");
  params.addRequiredParam<MaterialPropertyName>("p_vapor", "Pressure of the vapor phase");
  params.addRequiredParam<MaterialPropertyName>("vel_liquid", "Velocity of the liquid");
  params.addRequiredParam<MaterialPropertyName>("vel_vapor", "Velocity of the vapor");
  params.addRequiredParam<MaterialPropertyName>("v_liquid", "Specific volume of the liquid");
  params.addRequiredParam<MaterialPropertyName>("v_vapor", "Specific volume of the vapor");
  params.addRequiredParam<MaterialPropertyName>("e_liquid",
                                                "Specific internal energy of the liquid");
  params.addRequiredParam<MaterialPropertyName>("e_vapor", "Specific internal energy of the vapor");
  params.addRequiredParam<MaterialPropertyName>("T_liquid", "Temperature of the liquid");
  params.addRequiredParam<MaterialPropertyName>("T_vapor", "Temperature of the vapor");
  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T_sat_liquid",
                                                "Liquid saturation temperature property");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the 2-phase fluid properties user object");
  params.addCoupledVar("q_wall", "Wall heat flux");
  params.addRequiredParam<UserObjectName>(
      "chf_table", "The name of the user object name with critical heat flux table");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");
  return params;
}

WallHeatTransferCoefficient7EqnBaseMaterial::WallHeatTransferCoefficient7EqnBaseMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw_liquid(declareProperty<Real>("Hw_liquid")),
    _Hw_vapor(declareProperty<Real>("Hw_vapor")),

    _ht_geom(THM::stringToEnum<PipeBase::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),
    _PoD(getParam<Real>("PoD")),
    _alpha_v_min(getParam<Real>("alpha_v_min")),
    _alpha_v_max(getParam<Real>("alpha_v_max")),
    _arhoA_liquid(coupledValue("arhoA_liquid")),
    _arhoA_vapor(coupledValue("arhoA_vapor")),
    _area(coupledValue("A")),
    _D_h(coupledValue("D_h")),
    _alpha_liquid(getMaterialProperty<Real>("alpha_liquid")),
    _alpha_vapor(getMaterialProperty<Real>("alpha_vapor")),
    _rho_liquid(getMaterialProperty<Real>("rho_liquid")),
    _rho_vapor(getMaterialProperty<Real>("rho_vapor")),
    _p_liquid(getMaterialProperty<Real>("p_liquid")),
    _p_vapor(getMaterialProperty<Real>("p_vapor")),
    _vel_liquid(getMaterialProperty<Real>("vel_liquid")),
    _vel_vapor(getMaterialProperty<Real>("vel_vapor")),
    _v_liquid(getMaterialProperty<Real>("v_liquid")),
    _v_vapor(getMaterialProperty<Real>("v_vapor")),
    _e_liquid(getMaterialProperty<Real>("e_liquid")),
    _e_vapor(getMaterialProperty<Real>("e_vapor")),
    _T_liquid(getMaterialProperty<Real>("T_liquid")),
    _T_vapor(getMaterialProperty<Real>("T_vapor")),
    _T_wall(getMaterialProperty<Real>("T_wall")),
    _T_sat_liquid(getMaterialProperty<Real>("T_sat_liquid")),
    _has_q_wall(isParamValid("q_wall")),
    _q_wall(_has_q_wall ? &coupledValue("q_wall") : nullptr),
    _gravity_magnitude(getParam<Real>("gravity_magnitude")),
    _fp(getUserObject<TwoPhaseFluidProperties>("fp")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getVaporName())),
    _liquid_props(getUserObjectByName<LiquidFluidPropertiesInterface>(_fp.getLiquidName())),
    _chf_table(getUserObject<CHFTable>("chf_table"))
{
  if (_ht_geom == PipeBase::ROD_BUNDLE && _PoD == 0.)
    mooseError(name(), ": When using ROD_BUNDLE geometry, you have to provide the PoD parameter.");
}
