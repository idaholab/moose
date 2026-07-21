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
#include "SBMSurfaceMeshBuilder.h"
#include "PointInPolyhedronCheck.h"

// Performs point-in-polyhedron checks using a single surface mesh
class PointInPolyhedronCheckUO : public PointInPolyhedronBaseUO
{
public:
  static InputParameters validParams();
  PointInPolyhedronCheckUO(const InputParameters & parameters);

  virtual void initialSetup() override;
  /// Main function: Determine if a point is inside the geometry
  virtual bool ifInside(const Point & p) const
  {
    /// TODO: We mark the point on the surface as inside now
    return _in_out_test_struct->sideness(p) != SurfaceSide::OUTSIDE;
  }

protected:
  const SBMSurfaceMeshBuilder & _builder;

  /// Smart pointer to PointInPolyhedronCheck, valid only during this object's lifetime.
  std::unique_ptr<PointInPolyhedronCheck> _in_out_test_struct;
};
