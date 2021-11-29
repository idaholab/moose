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

class PointSamplerBase : public GeneralVectorPostprocessor,
                         public CoupleableMooseVariableDependencyIntermediateInterface,
                         public MooseVariableInterface<Real>,
                         protected SamplerBase
{
public:
  static InputParameters validParams();

  PointSamplerBase(const InputParameters & parameters);

  virtual ~PointSamplerBase() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  void setPointsVector(const std::vector<Point> & points);
  void transferPointsVector(std::vector<Point> && points);

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

  unsigned int _qp;

  std::unique_ptr<PointLocatorBase> _pl;

  /// Postprocessor multiplying the variables
  const Real & _pp_value;

  /// Whether to return a warning if a discontinuous variable is sampled on a face
  const bool _warn_discontinuous_face_values;

  /// Whether discontinuous variable values are requested, which are ill-defined on faces for this object
  bool _discontinuous_variables;
};
