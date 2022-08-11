//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "InputParameters.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

class MooseObject;

/**
 * Defines a discretized line segment in 3D space
 */
class DiscreteLineSegmentInterface
{
public:
  DiscreteLineSegmentInterface(const MooseObject * moose_object);

  virtual Point getPosition() const { return _position; }
  virtual RealVectorValue getDirection() const { return _dir; }
  virtual Real getRotation() const { return _rotation; }

  virtual Real getNumElems() const { return _n_elem; }
  virtual Real getLength() const { return _length; }

  /*
   * Computes the axial coordinate for a given point in 3-D space.
   *
   * @param[in] p   Point in 3-D space
   */
  Real computeAxialCoordinate(const Point & p) const;

  /**
   * Computes point in 3-D space from a point in reference space.
   *
   * @param[in] p   Point in reference space
   */
  Point computeRealPointFromReferencePoint(const Point & p) const;

  /**
   * Computes point in reference space from a point in 3-D space.
   *
   * @param[in] p   Point in 3-D space
   */
  Point computeReferencePointFromRealPoint(const Point & p) const;

protected:
  /**
   * Computes the direction transformation tensor
   */
  RealTensorValue computeDirectionTransformationTensor() const;

  /**
   * Computes the rotation transformation tensor
   */
  RealTensorValue computeXRotationTransformationTensor() const;

  /// Start position of axis in 3-D space
  const Point & _position;
  /// Unnormalized direction of axis from start position to end position
  const RealVectorValue & _dir_unnormalized;
  /// Normalized direction of axis from start position to end position
  const RealVectorValue _dir;
  /// Angle of rotation about the x-axis
  const Real & _rotation;

  /// Length of each axial section
  std::vector<Real> _lengths;
  /// Total axial length
  Real _length;

  /// Number of elements in each axial section
  const std::vector<unsigned int> & _n_elems;
  /// Total number of axial elements
  const unsigned int _n_elem;

  /// Number of axial sections
  const unsigned int _n_sections;

  /// Direction transformation tensor
  const RealTensorValue _R;
  /// Rotational transformation tensor about x-axis
  const RealTensorValue _Rx;

  /// Inverse direction transformation tensor
  const RealTensorValue _R_inv;
  /// Inverse rotational transformation tensor about x-axis
  const RealTensorValue _Rx_inv;

  /// Name of the MOOSE object
  const std::string _moose_object_name_dlsi;

public:
  static InputParameters validParams();
};
