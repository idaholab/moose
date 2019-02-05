#include "WallFrictionChurchillMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"

registerMooseObject("THMApp", WallFrictionChurchillMaterial);

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

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addRequiredParam<Real>("roughness", "Surface roughness");
  params.declareControllable("roughness");
  return params;
}

WallFrictionChurchillMaterial::WallFrictionChurchillMaterial(const InputParameters & parameters)
  : WallFriction3EqnBaseMaterial(parameters)
{
}

void
WallFrictionChurchillMaterial::computeQpProperties()
{
  Real Re = THM::Reynolds(1, _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);

  const Real f_F = WallFriction::FanningFrictionFactorChurchill(Re, _roughness, _D_h[_qp]);

  _f_D[_qp] = WallFriction::DarcyFrictionFactor(f_F);
  _df_D_drhoA[_qp] = 0;
  _df_D_drhouA[_qp] = 0;
  _df_D_drhoEA[_qp] = 0;
}
