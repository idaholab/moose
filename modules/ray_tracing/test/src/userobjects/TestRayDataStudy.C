//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestRayDataStudy.h"

registerMooseObject("RayTracingTestApp", TestRayDataStudy);

InputParameters
TestRayDataStudy::validParams()
{
  auto params = LotsOfRaysRayStudy::validParams();
  params.addRequiredParam<unsigned int>("data_size", "Size for the test Ray data");
  params.addRequiredParam<unsigned int>("aux_data_size", "Size for the test aux Ray data");
  return params;
}

TestRayDataStudy::TestRayDataStudy(const InputParameters & parameters)
  : LotsOfRaysRayStudy(parameters),
    _data_size(getParam<unsigned int>("data_size")),
    _aux_data_size(getParam<unsigned int>("aux_data_size"))
{
  _data_indices.resize(_data_size);
  for (unsigned int i = 0; i < _data_size; ++i)
    _data_indices[i] = registerRayData("test_data_" + std::to_string(i));

  _aux_data_indices.resize(_aux_data_size);
  for (unsigned int i = 0; i < _aux_data_size; ++i)
    _aux_data_indices[i] = registerRayAuxData("test_aux_data_" + std::to_string(i));

  _actual_start_indices.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    _actual_start_indices[i] = registerRayAuxData("actual_start_" + std::to_string(i));
}

RayData
TestRayDataStudy::dataValue(const unsigned int i, const Ray & ray) const
{
  Point actual_start;
  actual_start(0) = ray.auxData(_actual_start_indices[0]);
  actual_start(1) = ray.auxData(_actual_start_indices[1]);
  actual_start(2) = ray.auxData(_actual_start_indices[2]);

  const Point bounding_box_centroid = 0.5 * (boundingBox().min() + boundingBox().max());
  const RayData data_start =
      (actual_start - bounding_box_centroid).norm() * (RayData)(i + 1) / (RayData)_data_size;

  return data_start + dataValueChange(i, ray.distance());
}

RayData
TestRayDataStudy::dataValueChange(const unsigned int i, const Real distance) const
{
  return 0.3 * (RayData)(i + 1) * (RayData)_data_size * distance;
}

RayData
TestRayDataStudy::auxDataValue(const unsigned int i, const Ray & ray) const
{
  Point actual_start;
  actual_start(0) = ray.auxData(_actual_start_indices[0]);
  actual_start(1) = ray.auxData(_actual_start_indices[1]);
  actual_start(2) = ray.auxData(_actual_start_indices[2]);

  return (actual_start - boundingBox().min()).norm() * (RayData)(i + 1) * (RayData)_data_size;
}

void
TestRayDataStudy::modifyRays()
{
  for (const std::shared_ptr<Ray> & ray : _rays)
  {
    for (unsigned int i = 0; i < 3; ++i)
      ray->auxData(_actual_start_indices[i]) = ray->currentPoint()(i);

    for (unsigned int i = 0; i < _data_size; ++i)
      ray->data(_data_indices[i]) = dataValue(i, *ray);

    for (unsigned int i = 0; i < _aux_data_size; ++i)
      ray->auxData(_aux_data_indices[i]) = auxDataValue(i, *ray);
  }
}

void
TestRayDataStudy::onCompleteRay(const std::shared_ptr<Ray> & ray)
{
  LotsOfRaysRayStudy::onCompleteRay(ray);

  const Ray & const_ray = *ray;

  for (unsigned int i = 0; i < _data_size; ++i)
    if (!MooseUtils::absoluteFuzzyEqual(const_ray.data(_data_indices[i]), dataValue(i, const_ray)))
      mooseError(_name, ": Incorrect expected data");

  for (unsigned int i = 0; i < _aux_data_size; ++i)
    if (!MooseUtils::absoluteFuzzyEqual(const_ray.auxData(_aux_data_indices[i]),
                                        auxDataValue(i, const_ray)))
      mooseError(_name, ": Incorrect expected aux data");
}
