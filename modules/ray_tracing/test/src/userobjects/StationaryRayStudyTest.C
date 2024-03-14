//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StationaryRayStudyTest.h"

#include "Function.h"

registerMooseObject("RayTracingTestApp", StationaryRayStudyTest);

InputParameters
StationaryRayStudyTest::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addParam<std::vector<FunctionName>>(
      "data_functions", {}, "Functions to use to set the data (if any)");
  params.addParam<std::vector<FunctionName>>(
      "aux_data_functions", {}, "Functions to use to set the aux data (if any)");

  params.set<bool>("_use_ray_registration") = false;

  return params;
}

StationaryRayStudyTest::StationaryRayStudyTest(const InputParameters & parameters)
  : RayTracingStudy(parameters)
{
  for (const auto & function : getParam<std::vector<FunctionName>>("data_functions"))
    _data_functions.emplace_back(registerRayData(function), &getFunctionByName(function));

  for (const auto & function : getParam<std::vector<FunctionName>>("aux_data_functions"))
    _aux_data_functions.emplace_back(registerRayAuxData(function), &getFunctionByName(function));
}

void
StationaryRayStudyTest::generateRays()
{
  const auto define_ray = [this](const auto elem, const auto & start)
  {
    auto ray = acquireRay();
    ray->setStart(start, elem);
    ray->setStationary();
    for (const auto & [i, function] : _data_functions)
      ray->data(i) = function->value(_t, start);
    for (const auto & [i, function] : _aux_data_functions)
      ray->auxData(i) = function->value(_t, start);
    moveRayToBuffer(ray);
  };

  // elem centroids
  for (const Elem * elem : *_mesh.getActiveLocalElementRange())
    define_ray(elem, elem->vertex_average());
  // boundary side centroids
  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    const Elem * elem = bnd_elem->_elem;
    if (elem->processor_id() == _pid)
      define_ray(elem, elemSide(*elem, bnd_elem->_side).vertex_average());
  }
}
