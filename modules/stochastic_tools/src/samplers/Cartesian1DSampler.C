//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Cartesian1DSampler.h"

registerMooseObjectAliased("StochasticToolsApp", Cartesian1DSampler, "Cartesian1D");

InputParameters
Cartesian1DSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Provides complete Cartesian product for the supplied variables.");
  params.addRequiredParam<std::vector<Real>>(
      "linear_space_items",
      "A list of triplets, each item should include the min, step size, and number of steps.");
  params.addRequiredParam<std::vector<Real>>("nominal_values", "Nominal values for each column.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

Cartesian1DSampler::Cartesian1DSampler(const InputParameters & parameters)
  : Sampler(parameters), _nominal_values(getParam<std::vector<Real>>("nominal_values"))
{
  const std::vector<Real> & items = getParam<std::vector<Real>>("linear_space_items");
  if (items.size() % 3 != 0)
    paramError("linear_space_items",
               "The number of numeric items must be divisible by 3; min, max, divisions for each "
               "item are required.");
  const dof_id_type num_cols = items.size() / 3;
  if (_nominal_values.size() != num_cols)
    paramError("nominal_values",
               "The number of values specified must match the number of triplets in "
               "'linear_space_items'.");

  _grid_range.resize(num_cols + 1, 0);
  _grid_items.resize(num_cols);
  dof_id_type num_rows = 0;
  for (std::size_t i = 0; i < items.size(); i += 3)
  {
    if (items[i + 2] != std::floor(items[i + 2]))
      paramError("linear_space_items",
                 "The third entry for each item must be an integer; it provides the number of "
                 "entries in the resulting item vector.");

    if (items[i + 2] < 0)
      paramError("linear_space_items",
                 "The third entry for each item must be positive; it provides the number of "
                 "entries in the resulting item vector.");

    const dof_id_type col = i / 3;
    const dof_id_type ng = static_cast<dof_id_type>(items[i + 2]);

    num_rows += ng;
    _grid_range[col + 1] = num_rows;
    _grid_items[col].resize(ng);
    for (const auto & j : make_range(ng))
      _grid_items[col][j] = items[i] + j * items[i + 1];
  }

  setNumberOfRows(num_rows);
  setNumberOfCols(num_cols);
}

Real
Cartesian1DSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (row_index >= _grid_range[col_index] && row_index < _grid_range[col_index + 1])
    return _grid_items[col_index][row_index - _grid_range[col_index]];
  else
    return _nominal_values[col_index];
}
