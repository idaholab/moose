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

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addRequiredParam<Real>("roughness", "Surface roughness");
  return params;
}

WallFriction3EqnBaseMaterial::WallFriction3EqnBaseMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareProperty<Real>(_f_D_name)),
    _df_D_drhoA(declarePropertyDerivativeTHM<Real>(_f_D_name, "rhoA")),
    _df_D_drhouA(declarePropertyDerivativeTHM<Real>(_f_D_name, "rhouA")),
    _df_D_drhoEA(declarePropertyDerivativeTHM<Real>(_f_D_name, "rhoEA")),

    _mu(getMaterialProperty<Real>("mu")),
    _rho(coupledValue("rho")),
    _vel(coupledValue("vel")),
    _D_h(coupledValue("D_h")),
    _roughness(getParam<Real>("roughness"))
{
}
