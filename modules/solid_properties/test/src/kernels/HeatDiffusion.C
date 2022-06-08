#include "HeatDiffusion.h"

registerMooseObject("SolidPropertiesApp", HeatDiffusion);

template<>
InputParameters validParams<HeatDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Thermal conduction kernel");
  return params;
}


HeatDiffusion::HeatDiffusion(const InputParameters & parameters) :
    DerivativeMaterialInterface<Kernel>(parameters),
    _k(getMaterialProperty<Real>("k_solid")),
    _dk_dT(getMaterialPropertyDerivative<Real>("k_solid", _var.name()))
{
}

Real
HeatDiffusion::computeQpResidual()
{
  return _k[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
HeatDiffusion::computeQpJacobian()
{
  return (_k[_qp] * _grad_phi[_j][_qp] + _dk_dT[_qp] * _phi[_j][_qp] * _grad_u[_qp]) * _grad_test[_i][_qp];
}
