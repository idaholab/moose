//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RayDataValue.h"

// Local includes
#include "RayTracingStudy.h"

registerMooseObject("RayTracingApp", RayDataValue);

InputParameters
RayDataValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription(
      "Obtains a value from the data or aux data of a Ray after tracing has been completed.");

  params.addRequiredParam<UserObjectName>("study", "The RayTracingStudy that owns the Ray");
  params.addParam<RayID>("ray_id",
                         "The ID of the Ray to get the value from. This or 'ray_id' must be set.");
  params.addParam<std::string>(
      "ray_name", "Name of the Ray to get the value from. This or 'ray_name' must be set.");
  params.addRequiredParam<std::string>("data_name", "The name of the data to extract from the Ray");
  params.addParam<bool>("aux", false, "Whether or not the data is an auxiliary value");

  return params;
}

RayDataValue::RayDataValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _study(getUserObject<RayTracingStudy>("study")),
    _aux(getParam<bool>("aux")),
    _ray_name(parameters.isParamSetByUser("ray_name") ? &getParam<std::string>("ray_name")
                                                      : nullptr),
    _ray_id(parameters.isParamSetByUser("ray_id") ? &getParam<RayID>("ray_id") : nullptr)
{
  if (_ray_id && _ray_name)
    paramError("ray_id", "Either 'ray_id' or 'ray_name' must be set, but not both");
  if (!_ray_id && !_ray_name)
    mooseError("Must have either the 'ray_id' or the 'ray_name' param set.");

  if (_ray_name && !_study.useRayRegistration())
    paramError("study",
               _study.typeAndName(),
               " does not support Ray registration.\n\nThis is controlled by the "
               "'_use_ray_registration' private param within the study.");

  if (!_study.bankRaysOnCompletion())
    paramError("study",
               _study.typeAndName(),
               " does not bank Rays on completion.\n\nThis is controlled by the "
               "'_bank_rays_on_completion' private param within the study.");
}

void
RayDataValue::initialize()
{
  // Get the index for the data on the ray requested
  const auto & data_name = getParam<std::string>("data_name");
  if (_aux)
    _ray_data_index = _study.getRayAuxDataIndex(data_name, /* graceful = */ true);
  else
    _ray_data_index = _study.getRayDataIndex(data_name, /* graceful = */ true);
  if (_ray_data_index == Ray::INVALID_RAY_DATA_INDEX)
    paramError("ray_name",
               "The ",
               _study.typeAndName(),
               " does not have Ray ",
               (_aux ? "aux " : ""),
               "data associated with the name '",
               data_name,
               "'");
}

Real
RayDataValue::getValue()
{
  RayID id;
  if (_ray_name)
  {
    id = _study.registeredRayID(*_ray_name, /* graceful = */ true);
    if (id == Ray::INVALID_RAY_ID)
      mooseError(type(),
                 " '",
                 name(),
                 "': Could not find a registered Ray with the name '",
                 *_ray_name,
                 "'");
  }
  else
    id = *_ray_id;

  // This gathers the value from the proc that killed the Ray we're looking for
  if (_aux)
    return _study.getBankedRayAuxData(id, _ray_data_index);
  else
    return _study.getBankedRayData(id, _ray_data_index);
}
