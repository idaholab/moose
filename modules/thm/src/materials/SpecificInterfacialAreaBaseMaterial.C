#include "SpecificInterfacialAreaBaseMaterial.h"

template <>
InputParameters
validParams<SpecificInterfacialAreaBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("alpha_vapor", "Volume fraction of the vapor phase");
  params.addRequiredCoupledVar("rho_liquid", "Liquid phase density");
  params.addRequiredCoupledVar("rho_vapor", "Vapor phase density");
  params.addRequiredCoupledVar("vel_liquid", "Liquid phase velocity");
  params.addRequiredCoupledVar("vel_vapor", "Vapor phase velocity");
  params.addRequiredCoupledVar("T_liquid", "Liquid phase temperature");
  params.addRequiredCoupledVar("T_vapor", "Vapor phase temperature");
  params.addRequiredCoupledVar("p_liquid", "Liquid phase pressure");
  params.addRequiredCoupledVar("p_vapor", "Vapor phase pressure");
  params.addRequiredCoupledVar("D_h", "hydraulic diameter");
  params.addRequiredCoupledVar("arhou_liquid", "Liquid phase mass flux");
  params.addRequiredCoupledVar("arhou_vapor", "Vapor phase mass flux");

  params.addRequiredParam<MaterialPropertyName>("alpha_liquid",
                                                "Liquid volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("mu_liquid",
                                                "Liquid dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_vapor",
                                                "Vapor dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Surface tension material property");

  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");

  return params;
}

SpecificInterfacialAreaBaseMaterial::SpecificInterfacialAreaBaseMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _alpha_vapor(coupledValue("alpha_vapor")),
    _rho_l(coupledValue("rho_liquid")),
    _rho_v(coupledValue("rho_vapor")),
    _T_l(coupledValue("T_liquid")),
    _T_v(coupledValue("T_vapor")),
    _p_l(coupledValue("p_liquid")),
    _p_v(coupledValue("p_vapor")),
    _vel_l(coupledValue("vel_liquid")),
    _vel_v(coupledValue("vel_vapor")),
    _D_h(coupledValue("D_h")),
    _arhou_l(coupledValue("arhou_liquid")),
    _arhou_v(coupledValue("arhou_vapor")),

    _alpha_liquid(getMaterialProperty<Real>("alpha_liquid")),
    _dalpha_liquid_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha_liquid", "beta")),
    _mu_l(getMaterialProperty<Real>("mu_liquid")),
    _mu_v(getMaterialProperty<Real>("mu_vapor")),
    _surface_tension(getMaterialProperty<Real>("surface_tension")),

    _gravity_magnitude(getParam<Real>("gravity_magnitude")),

    _A_int(declareProperty<Real>("A_int")),
    _dA_int_dbeta(declarePropertyDerivativeTHM<Real>("A_int", "beta"))
{
}
