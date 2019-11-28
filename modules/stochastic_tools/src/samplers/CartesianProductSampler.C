//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianProductSampler.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", CartesianProductSampler);

defineLegacyParams(CartesianProductSampler);

InputParameters
CartesianProductSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Provides complete cartesian product for the supplied variables.");
  params.addRequiredParam<std::vector<Real>>(
      "items", "A list of items, each item should include a min, step size, and number of steps.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

CartesianProductSampler::CartesianProductSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  const std::vector<Real> & items = getParam<std::vector<Real>>("items");
  if (items.size() % 3 != 0)
    paramError("items",
               "The number of numeric items must be divisible by 3; min, max, divisions for each "
               "item are required.");

  dof_id_type n_rows = 1;
  for (std::size_t i = 0; i < items.size(); i += 3)
  {
    if (items[i + 2] != std::floor(items[i + 2]))
      paramError("items", "The third entry for each item must be an integer.");

    if (items[i + 2] < 0)
      paramError("items", "The third entry for each item must be positive.");

    unsigned int div = static_cast<unsigned int>(items[i + 2]);
    n_rows *= div;

    _grid_items.emplace_back(std::vector<Real>(div));
    for (std::size_t j = 0; j < _grid_items.back().size(); ++j)
      _grid_items.back()[j] = items[i] + j * items[i + 1];
  }

  setNumberOfRows(n_rows);
  setNumberOfCols(_grid_items.size());
}

void
CartesianProductSampler::sampleSetUp()
{
  dof_id_type d = 1;
  for (std::vector<std::vector<Real>>::const_reverse_iterator iter = _grid_items.rbegin();
       iter != _grid_items.rend();
       ++iter)
  {
    std::size_t n = iter->size();
    _denomenators.push_front(d);
    _moduli.push_front(n);
    d *= n;
  }
}

Real
CartesianProductSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _grid_items[col_index][(row_index / _denomenators[col_index]) % _moduli[col_index]];
}
