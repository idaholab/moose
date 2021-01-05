//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Metropolis.h"
#include "Distribution.h"
#include "Normal.h"

registerMooseObject("StochasticToolsApp", Metropolis);

InputParameters
Metropolis::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Markov Chain Monte Carlo sampling using Metropolis algorithm.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of Markov chains.");
  params.addRequiredParam<std::vector<DistributionName>>("distributions",
                                                         "The distribution names to be sampled.");
  params.addRequiredParam<std::vector<Real>>("proposal_std",
                                             "Standard deviations of the proposal distributions.");
  params.addRequiredParam<std::vector<Real>>(
      "initial_values", "Seed input values to get the Metropolis sampler started.");
  return params;
}

Metropolis::Metropolis(const InputParameters & parameters)
  : Sampler(parameters),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));
  if (_distributions.size() != _proposal_std.size() ||
      _distributions.size() != _initial_values.size())
    paramError("distributions",
               "The number of distributions, input names, proposal standard deviations, and "
               "initial values should be equal");

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _prev_val.resize(_distributions.size());
  for (unsigned i = 0; i < _distributions.size(); ++i)
    _prev_val[i].resize(getParam<dof_id_type>("num_rows"));
  _check_step = 0;
}

Real
Metropolis::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  if (_step > 1)
  {
    if (_check_step != _step && col_index == 0)
    {
      Real proposed_sample;
      Real acceptance_ratio;
      for (unsigned i = 0; i < _distributions.size(); ++i)
      {
        for (unsigned j = 0; j < getParam<dof_id_type>("num_rows"); ++j)
        {
          proposed_sample = Normal::quantile(getRand(), _prev_val[i][j], _proposal_std[i]);
          acceptance_ratio = std::log(_distributions[i]->pdf(proposed_sample)) -
                             std::log(_distributions[i]->pdf(_prev_val[i][j]));
          if (acceptance_ratio > std::log(getRand()))
            _prev_val[i][j] = proposed_sample;
        }
      }
      _check_step = _step;
      return _prev_val[col_index][row_index];
    }
    else
      return _prev_val[col_index][row_index];
  }
  else
  {
    _prev_val[col_index][row_index] = _initial_values[col_index];
    _check_step = _step;
    return _initial_values[col_index];
  }
}
