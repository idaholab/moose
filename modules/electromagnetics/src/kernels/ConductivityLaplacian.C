#include "ConductivityLaplacian.h"
#include "MooseMesh.h"

registerMooseObject("ElkApp", ConductivityLaplacian);

defineLegacyParams(ConductivityLaplacian);

InputParameters
ConductivityLaplacian::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Computes residual and Jacobian contribution for weak form term "
      "associated with $\\nabla \\cdot (\\sigma \\nabla V)$, where "
      "$\\sigma$ is the electrical conductivity and $V$ is the electrostatic potential.");
  params.addParam<MaterialPropertyName>(
      "conductivity_coefficient",
      "conductivity",
      "Property name of the material conductivity (Default: conductivity).");
  params.addParam<MaterialPropertyName>("conductivity_coefficient_dT",
                                        "conductivity_dT",
                                        "Property name of the derivative of the conductivity with "
                                        "respect to the temperature (Default: conductivity_dT).");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ConductivityLaplacian::ConductivityLaplacian(const InputParameters & parameters)
  : Kernel(parameters),
    _conductivity(getMaterialProperty<Real>("conductivity_coefficient")),
    _conductivity_dT(hasMaterialProperty<Real>("conductivity_coefficient_dT")
                         ? &getMaterialProperty<Real>("conductivity_coefficient_dT")
                         : nullptr)
{
}

Real
ConductivityLaplacian::computeQpResidual()
{
  return _conductivity[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
ConductivityLaplacian::computeQpJacobian()
{
  Real jac = _conductivity[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
  if (_conductivity_dT)
    jac += (*_conductivity_dT)[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
  return jac;
}
