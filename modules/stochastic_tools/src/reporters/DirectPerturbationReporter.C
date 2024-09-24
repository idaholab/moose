//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectPerturbationReporter.h"
#include "DirectPerturbationSampler.h"

registerMooseObject("StochasticToolsApp", DirectPerturbationReporter);

InputParameters
DirectPerturbationReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Compute local sensitivities using the direct perturbation method.");

  params.addRequiredParam<SamplerName>("sampler",
                                       "Direct PErturbation sampler used to generate samples.");
  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");
  params.addParam<std::vector<ReporterName>>(
      "reporters", {}, "List of Reporter values to utilized for statistic computations.");

  return params;
}

DirectPerturbationReporter::DirectPerturbationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters), _sampler(getSampler("sampler"))
{
  if (!dynamic_cast<DirectPerturbationSampler *>(&_sampler))
    paramError("sampler", "Computing Morris sensitivities requires the use of a Morris sampler.");

  if ((!isParamValid("reporters") && !isParamValid("vectorpostprocessors")) ||
      (getParam<std::vector<ReporterName>>("reporters").empty() &&
       getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors").empty()))
    mooseError(
        "The 'vectorpostprocessors' and/or 'reporters' parameters must be defined and non-empty.");
}

void
DirectPerturbationReporter::initialize()
{
}
