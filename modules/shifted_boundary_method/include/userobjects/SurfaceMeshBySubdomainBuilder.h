//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryMeshBuilder.h"

#include <unordered_map>

/**
 * BoundaryMeshBuilder specialization that groups the surface elements by
 * subdomain, building one SurfaceElementSet per subdomain instead of a single
 * whole-mesh set.
 */
class SurfaceMeshBySubdomainBuilder : public BoundaryMeshBuilder
{
public:
  static InputParameters validParams();
  SurfaceMeshBySubdomainBuilder(const InputParameters & parameters);

  /// Per-subdomain SurfaceElementSets. Valid after initialSetup().
  const std::unordered_map<subdomain_id_type, SurfaceElementSet> &
  getSurfaceElementSetsBySubdomain() const
  {
    return _sets_by_subdomain;
  }

protected:
  /// Build one SurfaceElementSet per subdomain instead of a whole-mesh set.
  virtual void buildDefaultSet() override;

  std::unordered_map<subdomain_id_type, SurfaceElementSet> _sets_by_subdomain;
};
