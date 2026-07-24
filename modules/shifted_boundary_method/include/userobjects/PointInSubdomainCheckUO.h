//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "libmesh/point.h"

#include "PointInPolyhedronBaseUO.h"
#include "SurfaceMeshBySubdomainBuilder.h"
#include "PointInPolyhedronCheck.h"

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

  const std::unordered_map<subdomain_id_type, std::unique_ptr<PointInPolyhedronCheck>> &
  getAllSubdomainCheckers() const
  {
    return _subdomain_id_checkers;
  }

protected:
  /// Builder providing subdomain-wise boundary elements
  const SurfaceMeshBySubdomainBuilder & _builder;

  /// Each subdomain has its own PointInPolyhedronCheck
  std::unordered_map<subdomain_id_type, std::unique_ptr<PointInPolyhedronCheck>>
      _subdomain_id_checkers;
};
