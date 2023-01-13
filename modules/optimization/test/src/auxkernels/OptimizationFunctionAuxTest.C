//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationFunctionAuxTest.h"
#include "OptimizationFunction.h"

registerMooseObject("OptimizationTestApp", OptimizationFunctionAuxTest);

InputParameters
OptimizationFunctionAuxTest::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addRequiredParam<FunctionName>("function", "Function evaluating parameter gradient.");
  return params;
}

OptimizationFunctionAuxTest::OptimizationFunctionAuxTest(const InputParameters & parameters)
  : ArrayAuxKernel(parameters),
    _func(dynamic_cast<const OptimizationFunction *>(&getFunction("function")))
{
  if (!_func)
    paramError("function", getParam<FunctionName>("function"), " is not an OptimizationFunction.");
}

RealEigenVector
OptimizationFunctionAuxTest::computeValue()
{
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];

  const std::vector<Real> pg = _func->parameterGradient(_t, p);
  if (pg.size() != _var.count())
    paramError("variable",
               "Number of components in array variable (",
               _var.count(),
               ") does not match number of parameters (",
               pg.size(),
               ").");

  RealEigenVector v(_var.count());
  for (unsigned int i = 0; i < _var.count(); ++i)
    v(i) = pg[i];
  return v;
}
