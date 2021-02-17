//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReflectRayBC.h"

// Local includes
#include "RayTracingStudy.h"

registerMooseObject("RayTracingApp", ReflectRayBC);

InputParameters
ReflectRayBC::validParams()
{
  auto params = GeneralRayBC::validParams();

  params.addParam<bool>(
      "warn_non_planar",
      true,
      "Whether or not to emit a warning if a Ray is being reflected on a non-planar side");

  params.addClassDescription("A RayBC that reflects a Ray in a specular manner on a boundary.");

  return params;
}

ReflectRayBC::ReflectRayBC(const InputParameters & params)
  : GeneralRayBC(params), _warn_non_planar(getParam<bool>("warn_non_planar"))
{
}

void
ReflectRayBC::onBoundary(const unsigned int num_applying)
{
  if (_warn_non_planar && _study.sideIsNonPlanar(_current_elem, _current_intersected_side))
    mooseWarning("A Ray is being reflected on a non-planar side.\n\n",
                 "Ray tracing on elements with non-planar faces is an approximation.\n\n",
                 "The normal used to compute the reflected direction is computed at\n",
                 "the side centroid and may not be valid for a non-planar side.\n\n",
                 "To disable this warning, set RayKernels/",
                 name(),
                 "/warn_non_planar=false.\n\n",
                 currentRay()->getInfo());

  // No need to do anything if the Ray's gonna die anyway
  if (!currentRay()->shouldContinue())
    return;

  // The direction this Ray reflects off this boundary
  const auto & normal = _study.getSideNormal(_current_elem, _current_intersected_side, _tid);
  const auto reflected_direction = reflectedDirection(currentRay()->direction(), normal);

  // Change it! Note here the usage of num_applying: if we are at a corner with a reflecting
  // boundary condition on both sides, we want to allow both boundary conditions to reflect the Ray.
  // Therefore, we skip the check that another RayBC has changed the Ray's trajectory when we are
  // applying multiple of the same ReflectRayBC at different boundaries at the same point to allow
  // this. Note that this double (or triple in 3D) reflection will only be allowed when the same
  // ReflectRayBC object is on both boundaries.
  changeRayDirection(reflected_direction, /* skip_changed_check = */ num_applying > 1);
}

Point
ReflectRayBC::reflectedDirection(const Point & direction, const Point & normal)
{
  mooseAssert(MooseUtils::absoluteFuzzyEqual(direction.norm(), 1.), "Direction not normalized");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(normal.norm(), 1.), "Normal not normalized");

  Point reflected_direction = direction;
  reflected_direction -= 2.0 * (reflected_direction * normal) * normal;
  return reflected_direction / reflected_direction.norm();
}
