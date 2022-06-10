//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayFunctionIC.h"
#include "Function.h"

registerMooseObject("MooseApp", ArrayFunctionIC);

InputParameters
ArrayFunctionIC::validParams()
{
  InputParameters params = ArrayInitialCondition::validParams();
  params.addRequiredParam<std::vector<FunctionName>>("function",
                                                     "The initial condition functions.");
  params.addClassDescription("An initial condition that uses a normal function of x, y, z to "
                             "produce values (and optionally gradients) for a field variable.");
  return params;
}

ArrayFunctionIC::ArrayFunctionIC(const InputParameters & parameters)
  : ArrayInitialCondition(parameters)
{
  auto & funcs = getParam<std::vector<FunctionName>>("function");
  if (_var.count() != funcs.size())
    mooseError("Number of functions does not agree with the number of array variable components");
  for (auto & func : funcs)
    _func.push_back(&getFunctionByName(func));
}

RealEigenVector
ArrayFunctionIC::value(const Point & p)
{
  RealEigenVector v(_var.count());
  for (unsigned int i = 0; i < _var.count(); ++i)
    v(i) = _func[i]->value(_t, p);
  return v;
}

RealVectorArrayValue
ArrayFunctionIC::gradient(const Point & p)
{
  RealVectorArrayValue v(_var.count(), LIBMESH_DIM);
  for (unsigned int i = 0; i < _var.count(); ++i)
  {
    auto gd = _func[i]->gradient(_t, p);
    for (const auto j : make_range(Moose::dim))
      v(i, j) = gd(j);
  }
  return v;
}
