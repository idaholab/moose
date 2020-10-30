//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionArrayAux.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionArrayAux);

InputParameters
FunctionArrayAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addClassDescription("Auxiliary Kernel that creates and updates an array field variable by "
                             "sampling functions through space and time.");
  params.addRequiredParam<std::vector<FunctionName>>("functions",
                                                     "The functions to use as the value");
  return params;
}

FunctionArrayAux::FunctionArrayAux(const InputParameters & parameters) : ArrayAuxKernel(parameters)
{
  auto & func_names = getParam<std::vector<FunctionName>>("functions");
  if (func_names.size() != _var.count())
    paramError("functions",
               "Number of functions must be equal to the number of components of array variable ",
               _var.name());

  for (auto & fname : func_names)
    _funcs.push_back(&getFunctionByName(fname));
}

RealEigenVector
FunctionArrayAux::computeValue()
{
  RealEigenVector v(_var.count());
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  for (unsigned int i = 0; i < _var.count(); ++i)
    v(i) = _funcs[i]->value(_t, p);
  return v;
}
