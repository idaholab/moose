#include "WallFrictionChurchillMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"

registerMooseObject("THMApp", WallFrictionChurchillMaterial);

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

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addRequiredParam<Real>("roughness", "Surface roughness");
  params.declareControllable("roughness");
  return params;
}

WallFrictionChurchillMaterial::WallFrictionChurchillMaterial(const InputParameters & parameters)
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
