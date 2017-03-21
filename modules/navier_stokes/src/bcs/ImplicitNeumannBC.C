/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ImplicitNeumannBC.h"

template <>
InputParameters
validParams<ImplicitNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription("This class implements a form of the Neumann boundary condition in "
                             "which the boundary term is treated 'implicitly'.");
  return params;
}

ImplicitNeumannBC::ImplicitNeumannBC(const InputParameters & parameters) : IntegratedBC(parameters)
{
}

Real
ImplicitNeumannBC::computeQpResidual()
{
  return _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
}

Real
ImplicitNeumannBC::computeQpJacobian()
{
  return (_grad_phi[_j][_qp] * _normals[_qp]) * _test[_i][_qp];
}

Real
ImplicitNeumannBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // off-diagonal derivatives are all zero.
  return 0.;
}
