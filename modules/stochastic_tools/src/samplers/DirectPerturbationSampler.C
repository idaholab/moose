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
  // TODO: we need to add capability to do absolute intervals too, in case there is a material
  // property with a nominal value of 0.0
  for (const auto interval : _relative_intervals)
    if (interval <= 0.0 || interval >= 1.0)
      paramError("relative_perturbation_intervals",
                 "The relative perturbation interval must be between 0 and 1!");

  // The number of samples will always include an addition sample for the reference point
  dof_id_type num_samples = 0;
  if (_perturbation_method == "central_difference")
    num_samples = 2 * _nominal_values.size() + 1;
  else
    num_samples = _nominal_values.size() + 1;

  // Adjusting the sample matrix
  setNumberOfRows(num_samples);
  setNumberOfCols(_nominal_values.size());

  _absolute_intervals = std::vector<Real>(num_samples - 1, 0);
  _parameter_vectors = std::vector<std::vector<Real>>(num_samples, _nominal_values);

  // Depending on what kind of perturbation we selected, the parameter values will change
  if (_perturbation_method == "central_difference")
    for (const auto i : index_range(_nominal_values))
    {
      _absolute_intervals[i] = _nominal_values[i] * _relative_intervals[i];
      // +1 because the first sample is the reference point
      _parameter_vectors[2 * i + 1][i] += _absolute_intervals[i] / 2;
      _parameter_vectors[2 * i + 2][i] -= _absolute_intervals[i] / 2;
    }
  else
    for (const auto i : index_range(_nominal_values))
    {
      _absolute_intervals[i] = _nominal_values[i] * _relative_intervals[i];
      _parameter_vectors[i + 1][i] += _absolute_intervals[i];
    }
}

Real
DirectPerturbationSampler::getAbsoluteInterval(const Real param_index) const
{
  mooseAssert(param_index < _absolute_intervals.size(),
              "We don't have the required absolute interval!");
  return _absolute_intervals[param_index];
}

Real
DirectPerturbationSampler::getRelativeInterval(const Real param_index) const
{
  mooseAssert(param_index < _absolute_intervals.size(),
              "We don't have the required relative interval!");
  return _relative_intervals[param_index];
}

Real
DirectPerturbationSampler::getNominalValue(const Real param_index) const
{
  mooseAssert(param_index < _nominal_values.size(),
              "We don't have the required nominal values for the given parameter!");
  return _nominal_values[param_index];
}

Real
DirectPerturbationSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _parameter_vectors[row_index][col_index];
}
