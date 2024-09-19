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
  params.addClassDescription("Sampler that reads samples from CSV file.");
  params.addRequiredParam<std::vector<Real>>(
      "nominal_parameter_values",
      "The nominal values of the parameters around which we shall perturb them.");
  params.addParam<std::vector<Real>>(
      "relative_perturbation_intervals", {0.1},
      "The numbers by which the nominal values are multiplied to get a perturbed value.");
  MooseEnum perturbation_type("central_difference forward_difference", "central_difference");
  params.addParam<MooseEnum>(
      "perturbation_method", perturbation_type,
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
  switch (_perturbation_method)
  {
    case "central_difference":
      num_samples = 2 * _nominal_values.size();
      break;
    case "forward_difference":
      num_samples = _nominal_values.size() + 1;
      break;
  }
  setNumberOfRows(_nominal_values.size());
  setNumberOfCols(num_samples);
}

Real
DirectPerturbationSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return 0.0; // entering samples into the matrix
}
