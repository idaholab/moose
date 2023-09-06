/// Considers cleavage plane anisotropy in the crack propagation

#include "ACInterfaceChangedVariable.h"

registerMooseObject("PhaseFieldApp", ACInterfaceChangedVariable);

InputParameters
ACInterfaceChangedVariable::validParams()
{
  InputParameters params = ACInterface::validParams();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel using a change of variable");
  params.addRequiredParam<MaterialPropertyName>(
      "order_parameter", "Order parameter material defnining the change of variable function");
  return params;
}

ACInterfaceChangedVariable::ACInterfaceChangedVariable(const InputParameters & parameters)
  : ACInterface(parameters),
    _dopdu(getMaterialPropertyDerivative<Real>("order_parameter", _var.name())),
    _d2opdu2(getMaterialPropertyDerivative<Real>("order_parameter", _var.name(), _var.name()))
{
}

void
ACInterfaceChangedVariable::initialSetup()
{
  validateCoupling<Real>("order_parameter");
}

Real
ACInterfaceChangedVariable::computeQpResidual()
{
  return ACInterface::computeQpResidual() * _dopdu[_qp];
}

Real
ACInterfaceChangedVariable::computeQpJacobian()
{
  return ACInterface::computeQpJacobian() * _dopdu[_qp] +
         ACInterface::computeQpResidual() * _d2opdu2[_qp] * _phi[_j][_qp];
}
