#include "InterfacialHeatTransferCoefBaseMaterial.h"
#include "HeatExchangeCoefficientPartitioning.h"
#include "FlowModel.h"
#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<InterfacialHeatTransferCoefBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<UserObjectName>(
      "hx_part",
      "The name of the user object that computes the heat exchange coefficient partitioning.");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("D_h", "hydraulic diameter");
  params.addRequiredParam<bool>("horizontal", "Is pipe horizontal");
  params.addRequiredParam<MooseEnum>(
      "ht_geom", PipeBase::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  params.addParam<Real>("PoD", 0, "Pitch-to-diameter ratio (needed for rod bundle).");
  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA_liquid", "Liquid mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_liquid", "Liquid momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_liquid", "Liquid energy equation variable: alpha*rho*E*A");
  params.addRequiredCoupledVar("arhoA_vapor", "Vapor mass equation variable: alpha*rho*A");
  params.addRequiredCoupledVar("arhouA_vapor", "Vapor momentum equation variable: alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA_vapor", "Vapor energy equation variable: alpha*rho*E*A");

  params.addRequiredCoupledVar("rho_liquid", "Liquid density");
  params.addRequiredCoupledVar("rho_vapor", "Vapor density");
  params.addRequiredCoupledVar("vel_liquid", "Liquid velocity");
  params.addRequiredCoupledVar("vel_vapor", "Vapor velocity");
  params.addRequiredCoupledVar("T_liquid", "Liquid temperature");
  params.addRequiredCoupledVar("T_vapor", "Vapor temperature");
  params.addRequiredCoupledVar("p_liquid", "Liquid pressure");
  params.addRequiredCoupledVar("p_vapor", "Vapor pressure");

  params.addRequiredParam<MaterialPropertyName>("A_int",
                                                "Interfacial area density material property");
  params.addRequiredParam<MaterialPropertyName>("T_int",
                                                "Interfacial temperature material property");
  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Surface tension material property");
  params.addRequiredParam<MaterialPropertyName>("alpha_liquid",
                                                "Liquid volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("alpha_vapor",
                                                "Vapor volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("k_liquid",
                                                "Liquid thermal conductivity material property");
  params.addRequiredParam<MaterialPropertyName>("k_vapor",
                                                "Vapor thermal conductivity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_liquid",
                                                "Liquid dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_vapor",
                                                "Vapor dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>(
      "cp_liquid", "Liquid constant-pressure specific heat material property");
  params.addRequiredParam<MaterialPropertyName>(
      "cp_vapor", "Vapor constant-pressure specific heat material property");
  params.addRequiredParam<MaterialPropertyName>("h_liquid",
                                                "Liquid specific enthalpy material property");
  params.addRequiredParam<MaterialPropertyName>("h_vapor",
                                                "Vapor specific enthalpy material property");
  params.addParam<MaterialPropertyName>("Gamma_wall", "Wall mass transfer material property");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the 2-phase fluid properties user object");

  return params;
}

InterfacialHeatTransferCoefBaseMaterial::InterfacialHeatTransferCoefBaseMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _area(coupledValue("A")),
    _D_h(coupledValue("D_h")),
    _is_horizontal(getParam<bool>("horizontal")),
    _ht_geom(THM::stringToEnum<PipeBase::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),
    _PoD(getParam<Real>("PoD")),

    _beta(coupledValue("beta")),
    _beta_dot(coupledDot("beta")),
    _arhoA_liquid(coupledValue("arhoA_liquid")),
    _arhouA_liquid(coupledValue("arhouA_liquid")),
    _arhoEA_liquid(coupledValue("arhoEA_liquid")),
    _arhoA_vapor(coupledValue("arhoA_vapor")),
    _arhouA_vapor(coupledValue("arhouA_vapor")),
    _arhoEA_vapor(coupledValue("arhoEA_vapor")),

    _alpha_liquid(getMaterialProperty<Real>("alpha_liquid")),
    _dalpha_liquid_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha_liquid", "beta")),
    _alpha_vapor(getMaterialProperty<Real>("alpha_vapor")),

    _rho_liquid(coupledValue("rho_liquid")),
    _rho_vapor(coupledValue("rho_vapor")),
    _vel_liquid(coupledValue("vel_liquid")),
    _vel_vapor(coupledValue("vel_vapor")),
    _T_liquid(coupledValue("T_liquid")),
    _T_vapor(coupledValue("T_liquid")),
    _p_liquid(coupledValue("p_liquid")),
    _p_vapor(coupledValue("p_vapor")),

    _surface_tension(getMaterialProperty<Real>("surface_tension")),
    _k_liquid(getMaterialProperty<Real>("k_liquid")),
    _k_vapor(getMaterialProperty<Real>("k_vapor")),
    _mu_liquid(getMaterialProperty<Real>("mu_liquid")),
    _mu_vapor(getMaterialProperty<Real>("mu_vapor")),
    _cp_liquid(getMaterialProperty<Real>("cp_liquid")),
    _cp_vapor(getMaterialProperty<Real>("cp_vapor")),
    _h_liquid(getMaterialProperty<Real>("h_liquid")),
    _h_vapor(getMaterialProperty<Real>("h_vapor")),

    _A_int(getMaterialProperty<Real>("A_int")),
    _dA_int_dbeta(getMaterialPropertyDerivativeTHM<Real>("A_int", "beta")),
    _T_int(getMaterialProperty<Real>("T_int")),

    _hx_part(getUserObject<HeatExchangeCoefficientPartitioning>("hx_part")),

    _gravity_magnitude(getParam<Real>("gravity_magnitude")),

    _fp(getUserObject<TwoPhaseFluidProperties>("fp")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_fp.getVaporName())),

    // Declare material properties
    _vhtc_liquid(declareProperty<Real>("vhtc_liquid")),
    _dvhtc_liquid_dbeta(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "beta")),
    _dvhtc_liquid_darhoAL(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhoA_liquid")),
    _dvhtc_liquid_darhouAL(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhouA_liquid")),
    _dvhtc_liquid_darhoEAL(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhoEA_liquid")),
    _dvhtc_liquid_darhoAV(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhoA_vapor")),
    _dvhtc_liquid_darhouAV(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhouA_vapor")),
    _dvhtc_liquid_darhoEAV(declarePropertyDerivativeTHM<Real>("vhtc_liquid", "arhoEA_vapor")),

    _vhtc_vapor(declareProperty<Real>("vhtc_vapor")),
    _dvhtc_vapor_dbeta(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "beta")),
    _dvhtc_vapor_darhoAL(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhoA_liquid")),
    _dvhtc_vapor_darhouAL(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhouA_liquid")),
    _dvhtc_vapor_darhoEAL(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhoEA_liquid")),
    _dvhtc_vapor_darhoAV(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhoA_vapor")),
    _dvhtc_vapor_darhouAV(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhouA_vapor")),
    _dvhtc_vapor_darhoEAV(declarePropertyDerivativeTHM<Real>("vhtc_vapor", "arhoEA_vapor"))
{
}

Real
InterfacialHeatTransferCoefBaseMaterial::getPartition()
{
  const Real alpha_liquid_dot = _dalpha_liquid_dbeta[_qp] * _beta_dot[_qp];
  return _hx_part.getPartition(_alpha_liquid[_qp], alpha_liquid_dot);
}

Real
InterfacialHeatTransferCoefBaseMaterial::getPartitionDer()
{
  const Real alpha_liquid_dot = _dalpha_liquid_dbeta[_qp] * _beta_dot[_qp];
  return _hx_part.getPartitionDer(_alpha_liquid[_qp], alpha_liquid_dot, _area[_qp]) *
         _dalpha_liquid_dbeta[_qp];
}

void
InterfacialHeatTransferCoefBaseMaterial::computeQpPropertiesZero()
{
  _vhtc_liquid[_qp] = 0;
  _vhtc_vapor[_qp] = 0;

  _dvhtc_liquid_dbeta[_qp] = 0;
  _dvhtc_liquid_darhoAL[_qp] = 0;
  _dvhtc_liquid_darhouAL[_qp] = 0;
  _dvhtc_liquid_darhoEAL[_qp] = 0;
  _dvhtc_liquid_darhoAV[_qp] = 0;
  _dvhtc_liquid_darhouAV[_qp] = 0;
  _dvhtc_liquid_darhoEAV[_qp] = 0;

  _dvhtc_vapor_dbeta[_qp] = 0;
  _dvhtc_vapor_darhoAL[_qp] = 0;
  _dvhtc_vapor_darhouAL[_qp] = 0;
  _dvhtc_vapor_darhoEAL[_qp] = 0;
  _dvhtc_vapor_darhoAV[_qp] = 0;
  _dvhtc_vapor_darhouAV[_qp] = 0;
  _dvhtc_vapor_darhoEAV[_qp] = 0;
}
