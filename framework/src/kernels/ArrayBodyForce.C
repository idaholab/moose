//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayBodyForce.h"

#include "Function.h"

registerMooseObject("MooseApp", ArrayBodyForce);

InputParameters
ArrayBodyForce::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredParam<std::vector<FunctionName>>("function", "The body force functions.");
  params.addClassDescription("Applies body forces specified with functions to an array variable.");
  return params;
}

ArrayBodyForce::ArrayBodyForce(const InputParameters & parameters) : ArrayKernel(parameters)
{
  auto & funcs = getParam<std::vector<FunctionName>>("function");
  if (_var.count() != funcs.size())
    paramError("function",
               "Number of functions must agree with the number of array variable components");
  for (auto & func : funcs)
    _func.push_back(&getFunctionByName(func));
}

void
ArrayBodyForce::computeQpResidual(RealEigenVector & residual)
{
  for (unsigned int p = 0; p < _count; ++p)
    residual(p) = -_test[_i][_qp] * _func[p]->value(_t, _q_point[_qp]);
}
