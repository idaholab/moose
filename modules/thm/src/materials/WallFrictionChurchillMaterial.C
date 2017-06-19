#include "WallFrictionChurchillMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"

template <>
InputParameters
validParams<WallFrictionChurchillMaterial>()
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

WallFrictionChurchillMaterial::WallFrictionChurchillMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Material>(parameters),
    _Cw_name(getParam<MaterialPropertyName>("Cw")),
    _Cw(declareProperty<Real>(_Cw_name)),
    _dCw_drhoA(declarePropertyDerivativeRelap<Real>(_Cw_name, "rhoA")),
    _dCw_drhouA(declarePropertyDerivativeRelap<Real>(_Cw_name, "rhouA")),
    _dCw_drhoEA(declarePropertyDerivativeRelap<Real>(_Cw_name, "rhoEA")),

    _mu(getMaterialProperty<Real>("mu")),
    _rho(coupledValue("rho")),
    _vel(coupledValue("vel")),
    _D_h(coupledValue("D_h")),
    _roughness(getParam<Real>("roughness"))
{
}

void
WallFrictionChurchillMaterial::computeQpProperties()
{
  Real Re = Reynolds(1, _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);

  _Cw[_qp] = WallFriction::Churchill(Re, _roughness, _D_h[_qp]) * 2. * _rho[_qp] / _D_h[_qp];
  _dCw_drhoA[_qp] = 0;
  _dCw_drhouA[_qp] = 0;
  _dCw_drhoEA[_qp] = 0;
}
