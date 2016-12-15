/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFracIntVar.h"

template<>
InputParameters validParams<PFFracIntVar>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription("Phase-field fracture residual for beta variable: Contribution from beta");

  return params;
}

PFFracIntVar::PFFracIntVar(const InputParameters & parameters):
  KernelValue(parameters)
{
}

Real
PFFracIntVar::precomputeQpResidual()
{
  // Residual is the variable value
  return _u[_qp];
}

Real
PFFracIntVar::precomputeQpJacobian()
{
  return _phi[_j][_qp];
}
