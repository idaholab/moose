#include "ReynoldsNumberMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", ReynoldsNumberMaterial);

template <>
InputParameters
validParams<ReynoldsNumberMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>("rho", "Density of the phase");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity of the phase");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity of the phase");
  params.addClassDescription("Computes Reynolds number as a material property");
  return params;
}

ReynoldsNumberMaterial::ReynoldsNumberMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rho(getMaterialProperty<Real>("rho")),
    _vel(getMaterialProperty<Real>("vel")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _mu(getMaterialProperty<Real>("mu")),
    _Re(declareProperty<Real>("Re"))
{
}

void
ReynoldsNumberMaterial::computeQpProperties()
{
  _Re[_qp] = THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);
}
