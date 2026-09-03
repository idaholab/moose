//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "SBMInterfaceManager.h"
#include "Ball.h"
#include "SBMSurfaceDistance.h"

#include "libmesh/mesh_tools.h"

#include <algorithm>
#include <cmath>
#include <iterator>

registerMooseObject("ShiftedBoundaryMethodApp", SBMInterfaceManager);

InputParameters
SBMInterfaceManager::validParams()
{
  InputParameters params = SurfaceMeshBySubdomainBuilder::validParams();
  params.renameParam(
      "complete_boundary_mesh",
      "complete_interface_mesh",
      "The saved mesh containing the complete boundary of each interface subdomain.");
  params.addRangeCheckedParam<unsigned int>(
      "leaf_max_size", 10, "leaf_max_size > 0", "Maximum number of points in a KDTree leaf.");
  params.addRangeCheckedParam<Real>(
      "tolerance",
      libMesh::TOLERANCE,
      "tolerance >= 0",
      "Relative distance tolerance for matching faces, scaled by the surface mesh size.");
  params.addRangeCheckedParam<Real>("normal_tolerance",
                                    1e-3,
                                    "normal_tolerance >= 0",
                                    "Tolerance for comparing unit face normals.");
  params.addClassDescription(
      "Detects interfaces between subdomains in one per-subdomain surface mesh and provides "
      "distance and normal queries for each interface.");
  return params;
}

SBMInterfaceManager::SBMInterfaceManager(const InputParameters & parameters)
  : SurfaceMeshBySubdomainBuilder(parameters),
    _leaf_max_size(getParam<unsigned int>("leaf_max_size")),
    _relative_distance_tolerance(getParam<Real>("tolerance")),
    _normal_tolerance(getParam<Real>("normal_tolerance"))
{
}

void
SBMInterfaceManager::initialSetup()
{
  SurfaceMeshBySubdomainBuilder::initialSetup();
  _distance_tolerance =
      _relative_distance_tolerance * MeshTools::create_bounding_box(mesh()).max_size();
  detectInterfaces();
}

void
SBMInterfaceManager::detectInterfaces()
{
  _interfaces.clear();
  const auto & sets = getSurfaceElementSetsBySubdomain();
  if (sets.size() < 2)
    mooseError("SBMInterfaceManager requires surfaces for at least two subdomains.");

  for (auto first_it = sets.begin(); first_it != sets.end(); ++first_it)
    for (auto second_it = std::next(first_it); second_it != sets.end(); ++second_it)
    {
      const auto first_id = first_it->first;
      const auto second_id = second_it->first;
      const auto key = std::minmax(first_id, second_id);
      const auto & source_set = key.first == first_id ? first_it->second : second_it->second;
      const auto & target_set = key.first == first_id ? second_it->second : first_it->second;

      Real target_max_radius = 0.0;
      for (const auto & element : target_set.elements())
        target_max_radius = std::max(target_max_radius, element->computeBoundingBall().radius());

      KDTree target_tree(target_set.centroids(), _leaf_max_size);
      auto & interface = _interfaces[{key.first, key.second}];
      std::vector<nanoflann::ResultItem<std::size_t, Real>> candidates;

      for (const auto & source : source_set.elements())
      {
        const Point centroid = source->elem().vertex_average();
        const Real search_radius =
            source->computeBoundingBall().radius() + target_max_radius + _distance_tolerance;
        candidates.clear();
        target_tree.radiusSearch(centroid, search_radius, candidates);

        const auto matched = std::find_if(
            candidates.begin(),
            candidates.end(),
            [&](const auto & candidate)
            {
              const auto & target = *target_set.elements()[candidate.first];
              if (std::abs(std::abs(source->normal() * target.normal()) - 1.0) > _normal_tolerance)
                return false;
              return SBMUtils::distanceFrom(target, centroid).norm() <= _distance_tolerance;
            });

        if (matched != candidates.end())
        {
          interface.elements.push_back(source.get());
          interface.centroids.push_back(centroid);
          interface.max_element_radius =
              std::max(interface.max_element_radius, source->computeBoundingBall().radius());
        }
      }

      if (interface.elements.empty())
        _interfaces.erase({key.first, key.second});
      else
        interface.kd_tree = std::make_unique<KDTree>(interface.centroids, _leaf_max_size);
    }
}

bool
SBMInterfaceManager::hasInterface(SubdomainID first, SubdomainID second) const
{
  const auto pair = std::minmax(first, second);
  return _interfaces.count({pair.first, pair.second});
}

SBMInterfaceManager::InterfaceQueryResult
SBMInterfaceManager::queryInterface(SubdomainID first,
                                    SubdomainID second,
                                    const Point & point) const
{
  const auto ordered = std::minmax(first, second);
  const auto interface_it = _interfaces.find({ordered.first, ordered.second});
  if (interface_it == _interfaces.end())
    mooseError("SBMInterfaceManager: interface (", first, ", ", second, ") was not detected.");

  const auto & interface = interface_it->second;
  std::vector<std::size_t> indices;
  interface.kd_tree->neighborSearch(point, 1, indices);
  std::size_t closest = indices.front();
  Point distance = SBMUtils::distanceFrom(*interface.elements[closest], point);

  std::vector<nanoflann::ResultItem<std::size_t, Real>> candidates;
  interface.kd_tree->radiusSearch(
      point, distance.norm() + interface.max_element_radius, candidates);
  for (const auto & candidate : candidates)
  {
    const Point candidate_distance =
        SBMUtils::distanceFrom(*interface.elements[candidate.first], point);
    if (candidate_distance.norm() < distance.norm())
    {
      closest = candidate.first;
      distance = candidate_distance;
    }
  }

  Point normal = interface.elements[closest]->normal();
  if (first > second)
    normal *= -1.0;
  return {distance, normal};
}
