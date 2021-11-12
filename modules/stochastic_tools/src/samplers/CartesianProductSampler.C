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

registerMooseObjectAliased("StochasticToolsApp", CartesianProductSampler, "CartesianProduct");
registerMooseObjectReplaced("StochasticToolsApp",
                            CartesianProductSampler,
                            "07/01/2020 00:00",
                            CartesianProduct);

InputParameters
CartesianProductSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Provides complete Cartesian product for the supplied variables.");
  params.addRequiredParam<std::vector<Real>>(
      "linear_space_items",
      "A list of triplets, each item should include the min, step size, and number of steps.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

CartesianProductSampler::CartesianProductSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  const std::vector<Real> & items = getParam<std::vector<Real>>("linear_space_items");
  if (items.size() % 3 != 0)
    paramError("linear_space_items",
               "The number of numeric items must be divisible by 3; min, max, divisions for each "
               "item are required.");

  std::vector<std::vector<Real>> grid_items;
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

    unsigned int div = static_cast<unsigned int>(items[i + 2]);
    grid_items.emplace_back(std::vector<Real>(div));
    for (std::size_t j = 0; j < grid_items.back().size(); ++j)
      grid_items.back()[j] = items[i] + j * items[i + 1];
  }

  _cp_ptr = std::make_unique<const StochasticTools::CartesianProduct<Real>>(grid_items);
  setNumberOfRows(_cp_ptr->numRows());
  setNumberOfCols(_cp_ptr->numCols());
}

Real
CartesianProductSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _cp_ptr->computeValue(row_index, col_index);
}
