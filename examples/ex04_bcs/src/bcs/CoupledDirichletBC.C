//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledDirichletBC.h"

registerMooseObject("ExampleApp", CoupledDirichletBC);

InputParameters
CoupledDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();

  // Specify input parameters that we want users to be able to set:
  params.addParam<Real>("alpha", 1.0, "Value multiplied by the coupled value on the boundary");
  params.addRequiredCoupledVar("some_var", "Value on the boundary");
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    // store the user-specified parameters from the input file...
    _alpha(getParam<Real>("alpha")),        // for the multiplier
    _some_var_val(coupledValue("some_var")) // for the coupled variable
{
}

Real
CoupledDirichletBC::computeQpResidual()
{
  // For dirichlet BCS, u=BC at the boundary, so the residual includes _u and the desired BC value:
  return _u[_qp] - (_alpha * _some_var_val[_qp]);
}
