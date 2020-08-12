#include "DarcyPressure.h"

registerMooseObject("BabblerApp", DarcyPressure);

InputParameters
DarcyPressure::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Compute the diffusion term for Darcy pressure ($p$) equation: "
                             "$-\\nabla \\cdot \\frac{\\mathbf{K}}{\\mu} \\nabla p = 0$");
  return params;
}

DarcyPressure::DarcyPressure(const InputParameters & parameters)
  : Kernel(parameters),

    // Set the coefficients for the pressure kernel
    _permeability(0.8451e-09),
    _viscosity(7.98e-04)
{
}

Real
DarcyPressure::computeQpResidual()
{
  return (_permeability / _viscosity) * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
DarcyPressure::computeQpJacobian()
{
  return (_permeability / _viscosity) * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
