//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UnsignedDistanceToSurfaceMesh.h"

class PointInPolyhedronCheckUO;

/**
 * Computes the signed distance to a surface mesh using KDTree nearest
 * neighbor lookup from SBMSurfaceMeshBuilder. The gradient returns the
 * unit vector pointing from the boundary element to the query point.
 */
class SignedDistanceToSurfaceMesh : public UnsignedDistanceToSurfaceMesh
{
public:
  SignedDistanceToSurfaceMesh(const InputParameters & parameters);

  static InputParameters validParams();

  void initialSetup() override;

  /// Return unsigned distance value
  Real value(Real t, const Point & p) const override;

private:
  /// user object for in-out test
  const PointInPolyhedronCheckUO * _in_out_test;
};
