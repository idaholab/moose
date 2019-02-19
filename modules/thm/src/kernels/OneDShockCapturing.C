#include "OneDShockCapturing.h"

registerMooseObject("THMApp", OneDShockCapturing);

template <>
InputParameters
validParams<OneDShockCapturing>()
{
  InputParameters params = validParams<OneDStabilizationBase>();
  return params;
}

OneDShockCapturing::OneDShockCapturing(const InputParameters & parameters)
  : OneDStabilizationBase(parameters)
{
}

Real
OneDShockCapturing::computeQpResidual()
{
  // delta * (grad(_u) * grad(phi))
  // See the compns notes document for description of this term.
  // Note: for systems with lots of variables, computeQpResidual() for shock-capturing is
  // always the same.

  // Using member function to compute delta
  // return delta() * (_grad_u[_qp] * _grad_test[_i][_qp]);

  // Using coupled aux variable.  delta doesn't work as a nodal aux because it depends
  // on solution gradients!  As an elemental aux, the first timestep doesn't converge.
  // return _delta[_qp] * (_grad_u[_qp] * _grad_test[_i][_qp]);

  // Use material property value of delta
  return _delta_matprop[_qp] * (_grad_u[_qp] * _grad_test[_i][_qp]);
}

Real
OneDShockCapturing::computeQpJacobian()
{
  // TODO - we compute the derivative of the operator (which gives us the symmetric
  // contribution, but neglect the derivative of delta.

  // Using member function to compute delta
  // return delta() * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);

  // Using coupled aux variable.  delta doesn't work as a nodal aux because it depends
  // on solution gradients!  As an elemental aux, the first timestep doesn't converge.
  // return _delta[_qp] * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);

  // Use material property value of delta
  return _delta_matprop[_qp] * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);
}

Real
OneDShockCapturing::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // TODO - delta has some dependence on rho*u, the off-diagonal variable.
  // At some point, we should actually compute this dependence...
  return 0.;
}
