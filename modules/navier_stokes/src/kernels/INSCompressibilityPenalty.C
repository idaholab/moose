/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSCompressibilityPenalty.h"

template <>
InputParameters
validParams<INSCompressibilityPenalty>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("The penalty term may be used when Dirichlet boundary condition is "
                             "applied to the entire boundary.");
  params.addParam<Real>("penalty", 1e-4, "Penalty value is used to relax the incompressibility");

  return params;
}

INSCompressibilityPenalty::INSCompressibilityPenalty(const InputParameters & parameters)
  : Kernel(parameters),
    // penalty value
    _penalty(getParam<Real>("penalty"))

{
}

Real
INSCompressibilityPenalty::computeQpResidual()
{
  // penalty*p*q
  return _penalty * _u[_qp] * _test[_i][_qp];
}

Real
INSCompressibilityPenalty::computeQpOffDiagJacobian(unsigned /* jvar */)
{
  // does not couple any variables
  return 0;
}

Real
INSCompressibilityPenalty::computeQpJacobian()
{
  // Derivative wrt to p
  return _penalty * _phi[_j][_qp] * _test[_i][_qp];
}
