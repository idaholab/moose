//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffTensorKernel.h"

registerMooseObject("MooseTestApp", DiffTensorKernel);

InputParameters
DiffTensorKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<FunctionName>("conductivity",
                                        "the name of the thermal conductivity function to utilize");
  return params;
}

DiffTensorKernel::DiffTensorKernel(const InputParameters & parameters)
  : Kernel(parameters), _k_comp(getFunction("conductivity"))
{
}

Real
DiffTensorKernel::computeQpResidual()
{
  RealTensorValue k = computeConductivity(_t, _qp);
  return k * _grad_test[_i][_qp] * _grad_u[_qp] - 4.0;
}

Real
DiffTensorKernel::computeQpJacobian()
{
  RealTensorValue k = computeConductivity(_t, _qp);
  return k * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}

RealTensorValue
DiffTensorKernel::computeConductivity(Real /*t*/, const Point & /*pt*/)
{
  // Return the value from the ParsedVectorFunction (this is the point of the test)
  RealVectorValue vector = _k_comp.vectorValue(_t, _qp);

  // Build tensor for thermal conductivity
  RealTensorValue tensor;
  tensor(0, 0) = vector(0);

#if LIBMESH_DIM > 1
  tensor(1, 1) = vector(1);
#endif
#if LIBMESH_DIM > 2
  tensor(2, 2) = vector(2);
#endif
  return tensor;
}
