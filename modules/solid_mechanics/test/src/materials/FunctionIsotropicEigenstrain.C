//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionIsotropicEigenstrain.h"
#include "RankTwoTensor.h"
#include "Function.h"

registerMooseObject("TensorMechanicsTestApp", FunctionIsotropicEigenstrain);

InputParameters
FunctionIsotropicEigenstrain::validParams()
{
  InputParameters params = ComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes an eigenstrain equal to the identity matrix times a "
                             "function. Also computes the gradient of that eigenstrain.");
  params.addParam<FunctionName>("function", "Function defining the eigenstrain");
  params.addParam<bool>("use_displaced_mesh", true, "Displaced mesh defaults to true");
  return params;
}

FunctionIsotropicEigenstrain::FunctionIsotropicEigenstrain(const InputParameters & parameters)
  : ComputeEigenstrainBase(parameters),
    _function(getFunction("function")),
    _eigenstrain_gradient(declareProperty<RankThreeTensor>(_eigenstrain_name + "_gradient"))
{
}

void
FunctionIsotropicEigenstrain::computeQpEigenstrain()
{
  RealVectorValue curr_point = _q_point[_qp];
  Real func_value = _function.value(_t, curr_point);
  RealGradient func_gradient = _function.gradient(_t, curr_point);

  RankTwoTensor I(RankTwoTensor::initIdentity);

  _eigenstrain[_qp] = func_value * I;
  _eigenstrain_gradient[_qp] = I.mixedProductJkI(func_gradient);
}
