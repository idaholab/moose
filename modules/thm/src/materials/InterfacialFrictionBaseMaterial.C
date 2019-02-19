#include "InterfacialFrictionBaseMaterial.h"
#include "Numerics.h"

template <>
InputParameters
validParams<InterfacialFrictionBaseMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("alpha_vapor", "Volume fraction of the vapor phase");
  params.addRequiredCoupledVar("rho_liquid", "Liquid phase density");
  params.addRequiredCoupledVar("rho_vapor", "Vapor phase density");
  params.addRequiredCoupledVar("vel_liquid", "Liquid phase velocity");
  params.addRequiredCoupledVar("vel_vapor", "Vapor phase velocity");
  params.addRequiredCoupledVar("p_liquid", "Liquid phase pressure");
  params.addRequiredCoupledVar("p_vapor", "Vapor phase pressure");
  params.addRequiredCoupledVar("T_liquid", "Liquid phase temperature");
  params.addRequiredCoupledVar("T_vapor", "Vapor phase temperature");
  params.addRequiredCoupledVar("D_h", "hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Surface tension material property");
  params.addRequiredParam<MaterialPropertyName>("mu_liquid",
                                                "Liquid dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("mu_vapor",
                                                "Vapor dynamic viscosity material property");
  params.addRequiredParam<MaterialPropertyName>("T_int",
                                                "Interfacial temperature material property");

  params.addRequiredParam<bool>("horizontal", "Is pipe horizontal");
  params.addRequiredParam<MooseEnum>(
      "ht_geom", PipeBase::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  params.addParam<Real>("PoD", 0, "Pitch-to-diameter ratio (needed for rod bundle).");

  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");

  params.addRequiredParam<UserObjectName>(
      "fp_liquid", "The name of the user object with liquid fluid properties");
  params.addRequiredParam<UserObjectName>(
      "fp_vapor", "The name of the user object with vapor fluid properties");

  return params;
}

InterfacialFrictionBaseMaterial::InterfacialFrictionBaseMaterial(const InputParameters & parameters)
  : Material(parameters),
    _is_horizontal(getParam<bool>("horizontal")),
    _ht_geom(THM::stringToEnum<PipeBase::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),
    _PoD(getParam<Real>("PoD")),
    _surface_tension(getMaterialProperty<Real>("surface_tension")),
    _mu_l(getMaterialProperty<Real>("mu_liquid")),
    _mu_v(getMaterialProperty<Real>("mu_vapor")),
    _TI(getMaterialProperty<Real>("T_int")),
    _alpha_vapor(coupledValue("alpha_vapor")),
    _rho_l(coupledValue("rho_liquid")),
    _rho_v(coupledValue("rho_vapor")),
    _T_l(coupledValue("T_liquid")),
    _T_v(coupledValue("T_vapor")),
    _v_l(coupledValue("vel_liquid")),
    _v_v(coupledValue("vel_vapor")),
    _p_l(coupledValue("p_liquid")),
    _p_v(coupledValue("p_vapor")),
    _D_h(coupledValue("D_h")),
    _gravity_magnitude(getParam<Real>("gravity_magnitude")),
    _fp_liquid(getUserObject<SinglePhaseFluidProperties>("fp_liquid")),
    _fp_vapor(getUserObject<SinglePhaseFluidProperties>("fp_vapor")),
    _f_i(declareProperty<Real>("f_int"))
{
  if (_ht_geom == PipeBase::ROD_BUNDLE && _PoD == 0.)
    mooseError(name(), ": When using ROD_BUNDLE geometry, you have to provide the PoD parameter.");
}
