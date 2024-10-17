//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseVariableInterface.h"
#include "SamplerBase.h"

/**
 * Base class for sampling objects (variables, functors etc) at points
 */
class PointSamplerBase : public GeneralVectorPostprocessor, protected SamplerBase
{
public:
  static InputParameters validParams();

  PointSamplerBase(const InputParameters & parameters);

  virtual ~PointSamplerBase() {}

  virtual void initialize();
  virtual void finalize();

protected:
  /**
   * Find the local element that contains the point.  This will attempt to use a cached element to
   * speed things up.
   *
   * @param p The point in physical space
   * @return The Elem containing the point or NULL if this processor doesn't contain an element that
   * contains this point.
   */
  const Elem * getLocalElemContainingPoint(const Point & p);

  /// The Mesh we're using
  MooseMesh & _mesh;

  /// The points to evaluate at
  std::vector<Point> _points;

  /// The ID to use for each point (yes, this is Real on purpose)
  std::vector<Real> _ids;

  /// Vector of values per point
  std::vector<std::vector<Real>> _point_values;

  /// Whether or not the Point was found on this processor (short because bool and char don't work with MPI wrappers)
  std::vector<short> _found_points;

  /// Point locator
  std::unique_ptr<libMesh::PointLocatorBase> _pl;

  /// Postprocessor multiplying the variables
  const Real & _pp_value;

  /// Whether to return a warning if a discontinuous variable is sampled on a face
  const bool _warn_discontinuous_face_values;

  /// Whether values are requested for objects that are discontinuous on faces
  bool _discontinuous_at_faces;
};
