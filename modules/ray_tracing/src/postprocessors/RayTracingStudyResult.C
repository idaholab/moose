//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RayTracingStudyResult.h"

// Local includes
#include "RayTracingStudy.h"
#include "ParallelRayStudy.h"

registerMooseObject("RayTracingApp", RayTracingStudyResult);

InputParameters
RayTracingStudyResult::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Gets a result from a RayTracingStudy.");

  params.addRequiredParam<UserObjectName>("study", "The RayTracingStudy to get results from");

  MooseEnum results(
      "total_rays_started total_processor_crossings max_processor_crossings total_distance");
  params.addRequiredParam<MooseEnum>("result", results, "The result to use");

  return params;
}

RayTracingStudyResult::RayTracingStudyResult(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _study(getUserObject<RayTracingStudy>("study")),
    _result(getParam<MooseEnum>("result"))
{
}

Real
RayTracingStudyResult::getValue()
{
  switch (_result)
  {
    case 0:
      return _study.parallelRayStudy().totalWorkCompleted();
      break;
    case 1:
      return _study.totalProcessorCrossings();
      break;
    case 2:
      return _study.maxProcessorCrossings();
      break;
    case 3:
      return _study.totalDistance();
      break;
    default:
      mooseError("Unknown result type ", _result, " in ", name());
  }
}
