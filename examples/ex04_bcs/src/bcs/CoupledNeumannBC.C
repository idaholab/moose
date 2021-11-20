//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledNeumannBC.h"

registerMooseObject("ExampleApp", CoupledNeumannBC);

InputParameters
CoupledNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();

  // Specify input parameters that we want users to be able to set:
  params.addParam<Real>("alpha", 1.0, "Value multiplied by the coupled value on the boundary");
  params.addRequiredCoupledVar("some_var", "Flux value at the boundary");
  return params;
}

CoupledNeumannBC::CoupledNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // store the user-specified parameters from the input file...
    _alpha(getParam<Real>("alpha")),        // for the multiplier
    _some_var_val(coupledValue("some_var")) // for the coupled variable
{
}

Real
CoupledNeumannBC::computeQpResidual()
{
  // For this Neumann BC grad(u)=alpha * v on the boundary.
  // We use the term produced from integrating the diffusion operator by parts.
  return -_test[_i][_qp] * _alpha * _some_var_val[_qp];
}
