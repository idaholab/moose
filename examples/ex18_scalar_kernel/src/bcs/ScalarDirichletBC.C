//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarDirichletBC.h"

registerMooseObject("ExampleApp", ScalarDirichletBC);

InputParameters
ScalarDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  // Here we are adding a parameter that will be extracted from the input file by the Parser
  params.addRequiredCoupledVar("scalar_var", "Value of the scalar variable");
  return params;
}

ScalarDirichletBC::ScalarDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),

    /**
     * Get a reference to the coupled variable's values.
     */
    _scalar_val(coupledScalarValue("scalar_var"))
{
}

Real
ScalarDirichletBC::computeQpResidual()
{
  // We coupled in a first order scalar variable, thus there is only one value in _scalar_val (and
  // it is - big surprise - on index 0)
  return _u[_qp] - _scalar_val[0];
}
