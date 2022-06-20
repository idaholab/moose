#include "HeatDiffusion.h"

registerMooseObject("SolidPropertiesTestApp", HeatDiffusion);

InputParameters
HeatDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Thermal conduction kernel");
  return params;
}


HeatDiffusion::HeatDiffusion(const InputParameters & parameters) :
    Kernel(parameters),
    _k(getMaterialProperty<Real>("k_solid")),
    _dk_dT(getMaterialProperty<Real>("dk_solid_dT"))
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
