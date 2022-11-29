//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseFunctionTabulate.h"
#include "PiecewiseBase.h"

registerMooseObject("MooseTestApp", PiecewiseFunctionTabulate);

InputParameters
PiecewiseFunctionTabulate::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tabulate the function nodes of a piecewise function, such as "
                             "PiecewiseLinear or PiecewiseConstant");
  params.addRequiredParam<FunctionName>(
      "function", "Name of the piecewise function object to extract the time steps from");
  return params;
}

PiecewiseFunctionTabulate::PiecewiseFunctionTabulate(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _piecewise_function(dynamic_cast<PiecewiseBase *>(
        &_fe_problem.getFunction(getParam<FunctionName>("function"),
                                 isParamValid("_tid") ? getParam<THREAD_ID>("_tid") : 0))),
    _x_col(declareVector("x")),
    _y_col(declareVector("y"))
{
  if (!_piecewise_function)
    paramError("function",
               "The supplied function must be derived from PiecewiseBase (e.g. PiecewiseLinear or "
               "PiecewiseConstant)");
}

void
PiecewiseFunctionTabulate::execute()
{
  auto size = _piecewise_function->functionSize();
  _x_col.resize(size);
  _y_col.resize(size);

  for (std::size_t i = 0; i < size; ++i)
  {
    _x_col[i] = _piecewise_function->domain(i);
    _y_col[i] = _piecewise_function->range(i);
  }
}
