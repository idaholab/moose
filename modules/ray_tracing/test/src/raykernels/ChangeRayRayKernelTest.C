//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChangeRayRayKernelTest.h"

#include "libmesh/remote_elem.h"

registerMooseObject("RayTracingTestApp", ChangeRayRayKernelTest);

InputParameters
ChangeRayRayKernelTest::validParams()
{
  auto params = GeneralRayKernel::validParams();
  params.addParam<std::string>("data_name", "Name of the data to change");
  params.addParam<Real>("add_value", 0, "Value to add to the data");
  params.addParam<Real>("scale_value", 1, "Value to scale the data with");

  params.addParam<bool>("change_start_direction",
                        false,
                        "Whether or not to change the Ray's start point and direction");
  params.addParam<bool>("change_direction_zero",
                        false,
                        "Whether or not to change the Ray's direction to the zero vector");
  params.addParam<bool>(
      "change_start_out_of_elem",
      false,
      "Whether or not to change the Ray's start point to a start out of the current element");

  return params;
}

ChangeRayRayKernelTest::ChangeRayRayKernelTest(const InputParameters & params)
  : GeneralRayKernel(params),
    _ray_data_index(isParamValid("data_name")
                        ? _study.getRayDataIndex(getParam<std::string>("data_name"))
                        : Ray::INVALID_RAY_DATA_INDEX),
    _add_value(getParam<Real>("add_value")),
    _scale_value(getParam<Real>("scale_value"))
{
  if (params.isParamSetByUser("add_value") && params.isParamSetByUser("scale_value"))
    mooseError("Cannot add and scale value");
}

void
ChangeRayRayKernelTest::onSegment()
{
  if (_ray_data_index != Ray::INVALID_RAY_DATA_INDEX)
  {
    currentRay()->data(_ray_data_index) *= _scale_value;
    currentRay()->data(_ray_data_index) += _add_value;
  }

  if (getParam<bool>("change_start_direction"))
    changeRayStartDirection(_current_elem->vertex_average(), -currentRay()->direction());
  if (getParam<bool>("change_direction_zero"))
    changeRayStartDirection(currentRay()->currentPoint(), Point(0, 0, 0));

  if (getParam<bool>("change_start_out_of_elem"))
    for (const auto neighbor : _current_elem->neighbor_ptr_range())
      if (neighbor && neighbor != remote_elem)
      {
        changeRayStartDirection(neighbor->vertex_average(), currentRay()->direction());
        break;
      }
}
