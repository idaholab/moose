//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RefractionRayKernelTest.h"

#include "RayTracingStudy.h"

registerMooseObject("RayTracingTestApp", RefractionRayKernelTest);

InputParameters
RefractionRayKernelTest::validParams()
{
  auto params = GeneralRayKernel::validParams();

  params.addRequiredCoupledVar("field", "The field variable that denotes the phase");

  params.addParam<Real>("r1", 1.0, "The first index of refraction");
  params.addParam<Real>("r2", 1.33, "The second index of refraction");

  return params;
}

RefractionRayKernelTest::RefractionRayKernelTest(const InputParameters & params)
  : GeneralRayKernel(params),
    _field(coupledValue("field")),
    _grad_field(coupledGradient("field")),
    _r1(getParam<Real>("r1")),
    _r2(getParam<Real>("r2")),
    _has_refracted_data_index(_study.registerRayData("has_refracted"))
{
}

void
RefractionRayKernelTest::onSegment()
{
  auto & has_refracted = currentRay()->data(_has_refracted_data_index);

  // If this Ray has refracted once already... don't let it refract again
  // This model is limited in the fact that it only checks the field change (if not 0 or 1) at the
  // center of the segment, therefore by this check it could refract multiple times when a field
  // changes over multiple elements. Don't allow this. This is good enough as a test object to test
  // the physical refraction.
  if (has_refracted)
    return;

  if (_field[0] > 0 && _field[0] < 1)
  {
    // Normal of the field is determined by the gradient
    const auto field_normal = _grad_field[0].unit();
    // Refract at the midpoint
    const auto refracted_point = 0.5 * (_current_segment_start + _current_segment_end);
    // Get the refracted direction
    const auto refracted_direction = refract(currentRay()->direction(), field_normal, _r1, _r2);

    // Refract!
    changeRayStartDirection(refracted_point, refracted_direction);

    // Note that this Ray has refracted so that it can't again. This is required because we don't
    // check the phase change in proper manner
    has_refracted = 1;
  }
}

Point
RefractionRayKernelTest::refract(const Point & direction,
                                 const Point & normal,
                                 const Real r1,
                                 const Real r2) const
{
  const Real c = std::abs(normal * direction);
  if (c > 1.0 - TOLERANCE) // parallel
    return direction;

  const Real r = r1 / r2;
  return (r * direction + (r * c - std::sqrt(1 - r * r * (1 - c * c))) * normal).unit();
}
