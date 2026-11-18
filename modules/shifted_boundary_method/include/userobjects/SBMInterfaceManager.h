//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "SurfaceMeshBySubdomainBuilder.h"
#include "KDTree.h"

#include <map>

/**
 * Detects shared interfaces between subdomains in a single surface mesh containing the
 * complete boundary of each subdomain, and provides distance and normal queries for each
 * interface.
 */
class SBMInterfaceManager : public SurfaceMeshBySubdomainBuilder
{
public:
  static InputParameters validParams();
  SBMInterfaceManager(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Result of querying distance and normal to an interface
  struct InterfaceQueryResult
  {
    /// Shortest distance vector to the interface
    RealVectorValue distance;
    /// Unit normal vector of the interface (pointing from first to second subdomain)
    RealVectorValue normal;
  };

  /// Whether an interface was detected between two subdomains.
  bool hasInterface(SubdomainID first, SubdomainID second) const;

  /// Return distance and normal data for an interface. The normal points from first to second.
  InterfaceQueryResult
  queryInterface(SubdomainID first, SubdomainID second, const Point & point) const;

protected:
  /// Geometric data for a detected interface between two subdomains
  struct InterfaceData
  {
    /// Surface elements belonging to the interface
    std::vector<const SurfaceElement *> elements;
    /// Centroids of the interface elements
    std::vector<Point> centroids;
    /// Spatial k-d tree built on element centroids for fast nearest-element queries
    std::unique_ptr<KDTree> kd_tree;
    /// Maximum bounding radius of elements on this interface
    Real max_element_radius = 0.0;
  };

  void detectInterfaces();

  /// Maximum number of points in a k-d tree leaf node
  const unsigned int _leaf_max_size;
  /// Relative tolerance for face matching, scaled by mesh size
  const Real _relative_distance_tolerance;
  /// Angular tolerance for comparing face normals
  const Real _normal_tolerance;
  /// Absolute distance tolerance for face matching
  Real _distance_tolerance = 0.0;
  /// Map of detected interfaces keyed by sorted subdomain ID pairs
  std::map<std::pair<SubdomainID, SubdomainID>, InterfaceData> _interfaces;
};
