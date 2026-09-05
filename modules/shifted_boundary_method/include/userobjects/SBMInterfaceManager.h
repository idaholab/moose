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
 * Detects interfaces in one surface mesh containing a complete surface for each subdomain.
 */
class SBMInterfaceManager : public SurfaceMeshBySubdomainBuilder
{
public:
  static InputParameters validParams();
  SBMInterfaceManager(const InputParameters & parameters);

  virtual void initialSetup() override;

  struct InterfaceQueryResult
  {
    RealVectorValue distance;
    RealVectorValue normal;
  };

  /// Whether an interface was detected between two subdomains.
  bool hasInterface(SubdomainID first, SubdomainID second) const;

  /// Return distance and normal data for an interface. The normal points from first to second.
  InterfaceQueryResult
  queryInterface(SubdomainID first, SubdomainID second, const Point & point) const;

protected:
  struct InterfaceData
  {
    std::vector<const SurfaceElement *> elements;
    std::vector<Point> centroids;
    std::unique_ptr<KDTree> kd_tree;
    Real max_element_radius = 0.0;
  };

  void detectInterfaces();

  const unsigned int _leaf_max_size;
  const Real _relative_distance_tolerance;
  const Real _normal_tolerance;
  Real _distance_tolerance = 0.0;
  std::map<std::pair<SubdomainID, SubdomainID>, InterfaceData> _interfaces;
};
