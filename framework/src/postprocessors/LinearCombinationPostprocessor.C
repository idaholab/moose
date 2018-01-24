//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearCombinationPostprocessor.h"

template <>
InputParameters
validParams<LinearCombinationPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  params.addRequiredParam<std::vector<PostprocessorName>>("pp_names", "List of post-processors");
  params.addRequiredParam<std::vector<Real>>(
      "pp_coefs", "List of linear combination coefficients for each post-processor");
  params.addParam<Real>("b", 0, "Additional value to add to sum");

  return params;
}

LinearCombinationPostprocessor::LinearCombinationPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _pp_names(getParam<std::vector<PostprocessorName>>("pp_names")),
    _n_pp(_pp_names.size()),
    _pp_coefs(getParam<std::vector<Real>>("pp_coefs")),
    _b_value(getParam<Real>("b"))
{
  if (_pp_coefs.size() != _n_pp)
    mooseError("The list parameters 'pp_names' and 'pp_coefs' must have the same length");

  _pp_values.resize(_n_pp);
  for (unsigned int i = 0; i < _n_pp; i++)
    _pp_values[i] = &getPostprocessorValueByName(_pp_names[i]);
}

void
LinearCombinationPostprocessor::initialize()
{
}

void
LinearCombinationPostprocessor::execute()
{
}

PostprocessorValue
LinearCombinationPostprocessor::getValue()
{
  Real linear_combination = _b_value;
  for (unsigned int i = 0; i < _n_pp; i++)
    linear_combination += _pp_coefs[i] * *(_pp_values[i]);

  return linear_combination;
}
