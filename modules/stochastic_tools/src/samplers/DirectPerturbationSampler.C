//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectPerturbationSampler.h"
#include "DelimitedFileReader.h"

registerMooseObject("StochasticToolsApp", DirectPerturbationSampler);

InputParameters
DirectPerturbationSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription(
      "Sampler that creates samples for a direct perturbation-based sensitivity study.");
  params.addRequiredParam<std::vector<Real>>(
      "nominal_parameter_values",
      "The nominal values of the parameters around which we shall perturb them.");
  params.addParam<std::vector<Real>>(
      "relative_perturbation_intervals",
      {0.01},
      "The numbers by which the nominal values are multiplied to get a perturbed value.");
  MooseEnum perturbation_type("central_difference forward_difference", "central_difference");
  params.addParam<MooseEnum>("perturbation_method",
                             perturbation_type,
                             "The perturbation method to use for creating samples.");

  return params;
}

DirectPerturbationSampler::DirectPerturbationSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _nominal_values(getParam<std::vector<Real>>("nominal_parameter_values")),
    _relative_intervals(getParam<std::vector<Real>>("relative_perturbation_intervals")),
    _perturbation_method(getParam<MooseEnum>("perturbation_method"))
{
  dof_id_type num_samples = 0;
  if (_perturbation_method == "central_difference")
    num_samples = 2 * _nominal_values.size();
  else
    num_samples = _nominal_values.size() + 1;

  // Adjusting the sample matrix
  setNumberOfRows(num_samples);
  setNumberOfCols(_nominal_values.size());

  _absolute_intervals = std::vector<Real>(num_samples, 0);
  _parameter_vectors = std::vector<std::vector<Real>>(num_samples, _nominal_values);

  // Depending on what kind of perturbation we selected, the parameter values will change
  if (_perturbation_method == "central_difference")
    for (const auto i : index_range(_nominal_values))
    {
      _absolute_intervals[i] = _nominal_values[i] * _relative_intervals[i];
      _parameter_vectors[2 * i][i] += _absolute_intervals[i] / 2;
      _parameter_vectors[2 * i + 1][i] -= _absolute_intervals[i] / 2;
    }
  else
    for (const auto i : index_range(_nominal_values))
    {
      _absolute_intervals[i] = _nominal_values[i] * _relative_intervals[i];
      _parameter_vectors[i + 1][i] += _absolute_intervals[i];
    }
}

Real
DirectPerturbationSampler::getInterval(const Real param_index) const
{
  mooseAssert(param_index < _absolute_intervals.size(), "We don't have the required interval!");
  return _absolute_intervals[param_index];
}

Real
DirectPerturbationSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _parameter_vectors[row_index][col_index];
}
