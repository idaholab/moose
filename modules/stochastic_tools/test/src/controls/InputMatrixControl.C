//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "InputMatrixControl.h"
#include "Function.h"

registerMooseObject("StochasticToolsTestApp", InputMatrixControl);

InputParameters
InputMatrixControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Controls the 'matrix' parameter of a InputMatrix sampler.");
  params.addRequiredParam<FunctionName>(
      "num_rows_function", "The function to use for controlling the number of matrix rows.");
  params.addRequiredParam<FunctionName>(
      "num_cols_function", "The function to use for controlling the number of matrix columns.");
  params.addRequiredParam<FunctionName>("sample_function",
                                        "The function to use for producing the samples; x and y "
                                        "will be the row and column index, respectively.");
  params.addRequiredParam<std::string>("parameter",
                                       "The InputMatrix 'matrix' parameter to control.");
  return params;
}

InputMatrixControl::InputMatrixControl(const InputParameters & parameters)
  : Control(parameters),
    _nrow_function(getFunction("num_rows_function")),
    _ncol_function(getFunction("num_cols_function")),
    _sample_function(getFunction("sample_function"))
{
}

void
InputMatrixControl::execute()
{
  const dof_id_type nrow = _nrow_function.value(_t);
  const dof_id_type ncol = _ncol_function.value(_t);

  RealEigenMatrix matrix(nrow, ncol);
  for (const auto i : make_range(nrow))
    for (const auto j : make_range(ncol))
      matrix(i, j) = _sample_function.value(_t, Point(i, j));

  _console << matrix << std::endl;
  setControllableValue<RealEigenMatrix>("parameter", matrix);
}
