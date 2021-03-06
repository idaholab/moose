#include "CoupledCoeffTimeDerivative.h"

registerMooseObject("ElkApp", CoupledCoeffTimeDerivative);

InputParameters
CoupledCoeffTimeDerivative::validParams()
{
  InputParameters params = CoupledTimeDerivative::validParams();
  params.addClassDescription(
      "Coupled time derivative kernel, multiplied by a user-specified coefficient.");
  params.addParam<Real>("coefficient", "User-provided coefficient for kernel");
  return params;
}

CoupledCoeffTimeDerivative::CoupledCoeffTimeDerivative(const InputParameters & parameters)
  : CoupledTimeDerivative(parameters), _coeff(getParam<Real>("coefficient"))
{
}

Real
CoupledCoeffTimeDerivative::computeQpResidual()
{
  return _coeff * CoupledTimeDerivative::computeQpResidual();
}

Real
CoupledCoeffTimeDerivative::computeQpJacobian()
{
  return _coeff * CoupledTimeDerivative::computeQpJacobian();
}
