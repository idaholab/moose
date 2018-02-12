#include "WallFrictionChurchillMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"

template <>
InputParameters
validParams<WallFrictionChurchillMaterial>()
{
  InputParameters params = validParams<WallFriction3EqnBaseMaterial>();
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
  : WallFriction3EqnBaseMaterial(parameters)
{
}

void
WallFrictionChurchillMaterial::computeQpProperties()
{
  Real Re = RELAP7::Reynolds(1, _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);

  _Cw[_qp] = WallFriction::Churchill(Re, _roughness, _D_h[_qp]) * 2. * _rho[_qp] / _D_h[_qp];
  _dCw_drhoA[_qp] = 0;
  _dCw_drhouA[_qp] = 0;
  _dCw_drhoEA[_qp] = 0;
}
