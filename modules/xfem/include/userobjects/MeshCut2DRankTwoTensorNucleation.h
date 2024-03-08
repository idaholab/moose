//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCut2DNucleationBase.h"
#include "RankTwoTensorForward.h"

class MeshCut2DRankTwoTensorNucleation : public MeshCut2DNucleationBase
{
public:
  static InputParameters validParams();

  MeshCut2DRankTwoTensorNucleation(const InputParameters & parameters);
  virtual ~MeshCut2DRankTwoTensorNucleation() {}

protected:
  /// scaling factor to extend the nucleated crack beyond the element edges.
  const Real _crack_length_scale;

  /// size of crack to nucleate, if unset, the nucleated crack is the length of one mesh element
  const Real _nucleation_length;

  /// The tensor from which the scalar quantity used as a nucleating criterion is extracted
  const MaterialProperty<RankTwoTensor> & _tensor;

  /// Threshold at which a crack is nucleated if exceeded
  const VariableValue & _nucleation_threshold;

  /// The type of scalar to be extracted from the tensor
  MooseEnum _scalar_type;

  /// Points used to define an axis of rotation for some scalar quantities
  const Point _point1;
  const Point _point2;

  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  virtual bool
  doesElementCrack(std::pair<RealVectorValue, RealVectorValue> & cutterElemNodes) override;

  // FIXME Lynn Copy from TraceRayTools.C in rayTracing module.  Remove once this function migrates
  // to libmesh.
  /// The standard "looser" tolerance to use in ray tracing when having difficulty finding intersection
  const Real TRACE_TOLERANCE = 1e-5;
  bool lineLineIntersect2D(const Point & start,
                           const Point & direction,
                           const Real length,
                           const Point & v0,
                           const Point & v1,
                           Point & intersection_point,
                           Real & intersection_distance);
};
