#include "WallFriction3EqnBaseMaterial.h"

template <>
InputParameters
validParams<WallFriction3EqnBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("rhoA", "Mass equation variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Momentum equation variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Total energy equation variable: rho*E*A");
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("vel", "x-component of the velocity");
  params.addRequiredCoupledVar("D_h", "hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("Cw", "Drag coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addRequiredParam<Real>("roughness", "Surface roughness");
  return params;
}

WallFriction3EqnBaseMaterial::WallFriction3EqnBaseMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _Cw_name(getParam<MaterialPropertyName>("Cw")),
    _Cw(declareProperty<Real>(_Cw_name)),
    _dCw_drhoA(declarePropertyDerivativeTHM<Real>(_Cw_name, "rhoA")),
    _dCw_drhouA(declarePropertyDerivativeTHM<Real>(_Cw_name, "rhouA")),
    _dCw_drhoEA(declarePropertyDerivativeTHM<Real>(_Cw_name, "rhoEA")),

    _mu(getMaterialProperty<Real>("mu")),
    _rho(coupledValue("rho")),
    _vel(coupledValue("vel")),
    _D_h(coupledValue("D_h")),
    _roughness(getParam<Real>("roughness"))
{
}
