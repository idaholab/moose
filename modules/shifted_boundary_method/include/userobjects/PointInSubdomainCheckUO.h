//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PointInPolyhedronBaseUO.h"
#include "SurfaceMeshBySubdomainBuilder.h"
#include "PointContainmentClassifier.h"

#include <map>

/// Performs in-out testing and identifies the subdomain containing a point.
class PointInSubdomainCheckUO : public PointInPolyhedronBaseUO
{
public:
  static InputParameters validParams();
  PointInSubdomainCheckUO(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Check if the point is inside any subdomain
  virtual bool ifInside(const Point & p) const;

  /// Determine which subdomain the point belongs to (returns "OUTSIDE" if none)
  virtual subdomain_id_type whichSubdomain(const Point & p) const;

  /// Read-only view of the per-subdomain checkers (subdomain id -> checker). Ownership stays with
  /// this object, so callers get const access and cannot mutate the checkers through it. Ordered by
  /// subdomain id, so iteration over the returned map is deterministic.
  const std::map<subdomain_id_type, std::unique_ptr<const PointContainmentClassifier>> &
  subdomainCheckers() const
  {
    return _subdomain_id_checkers;
  }

protected:
  /// Builder providing subdomain-wise SurfaceElementSets
  const SurfaceMeshBySubdomainBuilder & _builder;

  /// Each subdomain has its own PointContainmentClassifier (owns the checkers). Keyed and ordered
  /// by subdomain id, so iteration over it is deterministic. The checkers are const because they
  /// are never mutated after construction, which also keeps subdomainCheckers() read-only.
  std::map<subdomain_id_type, std::unique_ptr<const PointContainmentClassifier>>
      _subdomain_id_checkers;
};
