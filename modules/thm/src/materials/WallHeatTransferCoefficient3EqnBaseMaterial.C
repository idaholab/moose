#include "WallHeatTransferCoefficient3EqnBaseMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include <algorithm>

template <>
InputParameters
validParams<WallHeatTransferCoefficient3EqnBaseMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object with fluid properties");
  params.addRequiredParam<MooseEnum>(
      "ht_geom", PipeBase::getConvHeatTransGeometry("PIPE"), "Heat transfer geometry");
  params.addParam<Real>("PoD", 0, "Pitch-to-diameter ratio (needed for rod bundle).");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density of the liquid");
  params.addRequiredParam<MaterialPropertyName>("vel", "x-component of the liquid velocity");
  params.addRequiredCoupledVar("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("v", "Specific volume of the liquid");
  params.addRequiredParam<MaterialPropertyName>("e", "Specific internal energy of the liquid");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure of the liquid");
  params.addCoupledVar("q_wall", "Wall heat flux");

  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");

  return params;
}

WallHeatTransferCoefficient3EqnBaseMaterial::WallHeatTransferCoefficient3EqnBaseMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareProperty<Real>("Hw")),
    _ht_geom(THM::stringToEnum<PipeBase::EConvHeatTransGeom>(getParam<MooseEnum>("ht_geom"))),
    _PoD(getParam<Real>("PoD")),
    _rho(getMaterialProperty<Real>("rho")),
    _vel(getMaterialProperty<Real>("vel")),
    _D_h(coupledValue("D_h")),
    _v(getMaterialProperty<Real>("v")),
    _e(getMaterialProperty<Real>("e")),
    _T(getMaterialProperty<Real>("T")),
    _pressure(getMaterialProperty<Real>("p")),
    _has_q_wall(isParamValid("q_wall")),
    _q_wall(_has_q_wall ? &coupledValue("q_wall") : nullptr),
    _gravity_magnitude(getParam<Real>("gravity_magnitude")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  if (_ht_geom == PipeBase::ROD_BUNDLE && _PoD == 0.)
    mooseError(name(), ": When using ROD_BUNDLE geometry, you have to provide the PoD parameter.");
}
