#include "ODECoefTimeDerivative.h"

registerMooseObject("ThermalHydraulicsApp", ODECoefTimeDerivative);

InputParameters
ODECoefTimeDerivative::validParams()
{
  InputParameters params = ODETimeDerivative::validParams();
  params.addRequiredParam<Real>("coef", "The coefficient multiplying the time derivative term");
  params.addClassDescription("Time derivative term multiplied by a coefficient - used by ODEs.");
  params.declareControllable("coef");
  return params;
}

ODECoefTimeDerivative::ODECoefTimeDerivative(const InputParameters & parameters)
  : ODETimeDerivative(parameters), _coef(getParam<Real>("coef"))
{
}

Real
ODECoefTimeDerivative::computeQpResidual()
{
  return _coef * ODETimeDerivative::computeQpResidual();
}

Real
ODECoefTimeDerivative::computeQpJacobian()
{
  return _coef * ODETimeDerivative::computeQpJacobian();
}
