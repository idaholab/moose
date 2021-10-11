//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatinHypercubeSampler.h"
#include "Distribution.h"
registerMooseObjectAliased("StochasticToolsApp", LatinHypercubeSampler, "LatinHypercube");

InputParameters
LatinHypercubeSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Latin Hypercube Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The size of the square matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  return params;
}

LatinHypercubeSampler::LatinHypercubeSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  const auto & distribution_names = getParam<std::vector<DistributionName>>("distributions");
  for (const DistributionName & name : distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(distribution_names.size());
  setNumberOfRandomSeeds(2 * distribution_names.size());

  // The use of MooseRandom in this Sampler is fairly complex. There are two sets of random
  // generators. The first set (n = number columns) is used to generate the random probability
  // within each bin of the Latin hypercube sample. The second set (n) is used to shuffle the
  // probability values. Mainly due to how the shuffle operates, it is necessary for the management
  // of advancement of the generators to be handled manually.
  setAutoAdvanceGenerators(false);
}

void
LatinHypercubeSampler::sampleSetUp(const Sampler::SampleMode mode)
{
  // All calls to the generators occur in here. Calls to the random number generators
  // (i.e., getRand) are complete by the end of this function.

  // Flag to indicate what vector index to use in computeSample method
  _is_local = mode == Sampler::SampleMode::LOCAL;

  const Real bin_size = 1. / getNumberOfRows();
  _probabilities.resize(getNumberOfCols());
  if (mode == Sampler::SampleMode::GLOBAL)
  {
    for (dof_id_type col = 0; col < getNumberOfCols(); ++col)
    {
      std::vector<Real> & local = _probabilities[col];
      local.resize(getNumberOfRows());
      for (dof_id_type row = 0; row < getNumberOfRows(); ++row)
      {
        const auto lower = row * bin_size;
        const auto upper = (row + 1) * bin_size;
        local[row] = getRand(col) * (upper - lower) + lower;
      }
      shuffle(local, col + getNumberOfCols(), CommMethod::NONE);
    }
  }

  else
  {
    for (dof_id_type col = 0; col < getNumberOfCols(); ++col)
    {
      std::vector<Real> & local = _probabilities[col];
      local.resize(getNumberOfLocalRows());
      advanceGenerator(col, getLocalRowBegin());
      for (dof_id_type row = getLocalRowBegin(); row < getLocalRowEnd(); ++row)
      {
        const auto lower = row * bin_size;
        const auto upper = (row + 1) * bin_size;
        local[row - getLocalRowBegin()] = getRand(col) * (upper - lower) + lower;
      }
      advanceGenerator(col, getNumberOfRows() - getLocalRowEnd());

      // Do not advance generator for shuffle, the shuffle handles it
      shuffle(local, col + getNumberOfCols(), CommMethod::SEMI_LOCAL);
    }
  }
}

Real
LatinHypercubeSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // NOTE: All calls to generators (getRand, etc.) occur in sampleSetup
  auto row = _is_local ? row_index - getLocalRowBegin() : row_index;
  return _distributions[col_index]->quantile(_probabilities[col_index][row]);
}
