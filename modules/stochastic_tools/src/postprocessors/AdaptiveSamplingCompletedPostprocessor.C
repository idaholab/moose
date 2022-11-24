//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveSamplingCompletedPostprocessor.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", AdaptiveSamplingCompletedPostprocessor);

InputParameters
AdaptiveSamplingCompletedPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<SamplerName>(
      "sampler", "Sampler for  which we want to know if the sampling has been completed.");
  params.addClassDescription(
      "Informs whether a sampler has finished its sampling (1 = completed, 0 otherwise).");
  return params;
}

AdaptiveSamplingCompletedPostprocessor::AdaptiveSamplingCompletedPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _sampler(getSampler("sampler")), _sampling_completed(0)
{
}

void
AdaptiveSamplingCompletedPostprocessor::execute()
{
  _sampling_completed = _sampler.isAdaptiveSamplingCompleted();
}

void
AdaptiveSamplingCompletedPostprocessor::finalize()
{
  gatherMax(_sampling_completed);
}

Real
AdaptiveSamplingCompletedPostprocessor::getValue()
{
  return _sampling_completed;
}
