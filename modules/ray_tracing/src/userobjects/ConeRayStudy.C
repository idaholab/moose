//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConeRayStudy.h"

// Local includes
#include "RayTracingAngularQuadrature.h"

registerMooseObject("RayTracingApp", ConeRayStudy);

InputParameters
ConeRayStudy::validParams()
{
  auto params = RepeatableRayStudyBase::validParams();

  params.addClassDescription(
      "Ray study that spawns Rays in the direction of a cone from a given set of starting points.");

  params.addRequiredParam<std::vector<Point>>("start_points", "The point(s) of the cone(s).");
  params.addRequiredParam<std::vector<Point>>(
      "directions", "The direction(s) of the cone(s) (points down the center of each cone).");
  params.addParam<std::vector<Real>>("scaling_factors",
                                     "Scaling factors for each cone (if any). Defaults to 1.");

  params.addRequiredParam<std::vector<Real>>("half_cone_angles",
                                             "Angle of the half-cones in degrees (must be <= 90)");
  params.addParam<std::vector<unsigned int>>(
      "polar_quad_orders",
      "Order of the polar quadrature for each cone. Polar angle is between ray and the "
      "cone direction. Must be positive. This will default to 2 for all cones if not provided.");
  params.addParam<std::vector<unsigned int>>(
      "azimuthal_quad_orders",
      "Order of the azimuthal quadrature per quadrant for each cone. The azimuthal angle is "
      "measured in a plane perpendicular to the cone direction. This will default to 30 if not "
      "provided. Must be positive.");

  params.addRequiredParam<std::string>("ray_data_name",
                                       "The name of the Ray data that the angular quadrature "
                                       "weights and factors will be filled into for "
                                       "properly weighting the line source per Ray.");

  // Here we will not use Ray registration because we create so many Rays that
  // we don't want to keep track of them by name
  params.set<bool>("_use_ray_registration") = false;

  return params;
}

ConeRayStudy::ConeRayStudy(const InputParameters & parameters)
  : RepeatableRayStudyBase(parameters),
    _start_points(getParam<std::vector<Point>>("start_points")),
    _directions(getParam<std::vector<Point>>("directions")),
    _scaling_factors(isParamValid("scaling_factors")
                         ? getParam<std::vector<Real>>("scaling_factors")
                         : std::vector<Real>(_start_points.size(), 1)), // default to 1
    _half_cone_angles(getParam<std::vector<Real>>("half_cone_angles")),
    _polar_quad_orders(isParamValid("polar_quad_orders")
                           ? getParam<std::vector<unsigned int>>("polar_quad_orders")
                           : std::vector<unsigned int>(_start_points.size(), 2)), // default to 2
    _azimuthal_quad_orders(
        isParamValid("azimuthal_quad_orders")
            ? getParam<std::vector<unsigned int>>("azimuthal_quad_orders")
            : std::vector<unsigned int>(_start_points.size(), 30)), // default to 30
    _ray_data_index(registerRayData(getParam<std::string>("ray_data_name")))
{
  if (_directions.size() != _start_points.size())
    paramError("directions", "Not the same size as start_points.");

  if (_scaling_factors.size() != _start_points.size())
    paramError("scaling_factors", "Not the same size as start_points.");

  if (_half_cone_angles.size() != _start_points.size())
    paramError("half_cone_angles", "Not the same size as start_points.");
  for (const auto val : _half_cone_angles)
    if (val <= 0 || val > 90)
      paramError("half_cone_angles", "Must be > 0 and <= 90 degrees");

  if (_polar_quad_orders.size() != _start_points.size())
    paramError("polar_quad_orders", "Not the same size as start_points.");

  if (_azimuthal_quad_orders.size() != _start_points.size())
    paramError("azimuthal_quad_orders", "Not the same size as start_points.");

  if (_mesh.dimension() == 1)
    mooseError("Does not support 1D.");
}

void
ConeRayStudy::defineRays()
{
  // Loop through each cone
  for (std::size_t i = 0; i < _start_points.size(); ++i)
  {
    // Setup the angular quadrature and rotate it about the direction
    // (the direction points down the middle of the cone)
    RayTracingAngularQuadrature aq(_mesh.dimension(),
                                   _polar_quad_orders[i],
                                   _azimuthal_quad_orders[i],
                                   std::cos(_half_cone_angles[i] * M_PI / 180.),
                                   1);
    aq.rotate(_directions[i].unit());

    // For all angles in the angular quadrature, spawn a Ray
    for (std::size_t l = 0; l < aq.numDirections(); ++l)
    {
      // Get a Ray from the study to initialize
      std::shared_ptr<Ray> ray = acquireReplicatedRay();

      // Start from our cone point in the rotated angular quadrature direction
      // Note here that we do not need to set the starting element - all Rays
      // at this point are replicated across all processors and will be
      // properly claimed (moved to the starting proc with the correct starting elem)
      ray->setStart(_start_points[i]);
      ray->setStartingDirection(aq.getDirection(l));

      // Add the angular quadrature weight and scaling factor as data on the Ray for weighting
      //
      // In the 2D case, the 3D directions projected into the 2D plane may overlap. Therefore,
      // we could have multiple weights/sins for a single direction, which is why here we grab
      // the total weight.
      //
      // The angular quadrature weights sum to 2pi, so scale such that they scale to 1
      ray->data(_ray_data_index) = _scaling_factors[i] * aq.getTotalWeight(l) / (2. * M_PI);

      // Done with this Ray - move it to be traced later on
      _rays.emplace_back(std::move(ray));
    }
  }
}
