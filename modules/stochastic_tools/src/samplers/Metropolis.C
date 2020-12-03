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
  params.addRequiredParam<VectorPostprocessorName>(
      "inputs_vpp", "Vectorpostprocessor with the samples created by sampler.");
  params.addRequiredParam<std::vector<std::string>>(
      "inputs_names",
      "Name of vector from vectorpostprocessor with the samples created by sampler.");
  params.addRequiredParam<std::vector<Real>>("proposal_std",
                                             "Standard deviations of the proposal distributions.");
  params.addRequiredParam<std::vector<Real>>(
      "initial_values", "Seed input values to get the Metropolis sampler started.");
  return params;
}

Metropolis::Metropolis(const InputParameters & parameters)
  : Sampler(parameters),
    VectorPostprocessorInterface(this),
    _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  if (_distributions.size() != _inputs_names.size() ||
      _distributions.size() != _proposal_std.size() ||
      _distributions.size() != _initial_values.size())
    paramError("distributions",
               "The number of distributions, input names, proposal standard deviations, and "
               "initial values should be equal");

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
}

void
Metropolis::sampleSetUp()
{
  _values_distributed = isVectorPostprocessorDistributed("inputs_vpp");

  _inputs_ptr.clear();
  _inputs_ptr.reserve(_inputs_names.size());
  for (const auto & name : _inputs_names)
    _inputs_ptr.push_back(&getVectorPostprocessorValue("inputs_vpp", name, !_values_distributed));
}

Real
Metropolis::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step > 1)
  {
    Real proposed_sample = Normal::quantile(
      getRand(), (*_inputs_ptr[col_index])[row_index - offset], _proposal_std[col_index]);
    Real acceptance_ratio =
        std::log(_distributions[col_index]->pdf(proposed_sample)) -
        std::log(_distributions[col_index]->pdf((*_inputs_ptr[col_index])[row_index - offset]));
    if (acceptance_ratio > std::log(getRand()))
    {
      std::cout << proposed_sample << std::endl;
      return proposed_sample;
    }
    else
    {
      std::cout << (*_inputs_ptr[col_index])[row_index - offset] << std::endl;
      return (*_inputs_ptr[col_index])[row_index - offset];
    }
  }
  else
    return _initial_values[col_index];
}
