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

  /*
   * Computes the radial coordinate from the line axis for a given point in 3-D space.
   *
   * @param[in] p   Point in 3-D space
   */
  Real computeRadialCoordinate(const Point & p) const;

  /*
   * Gets the axial section index for a given point in 3-D space.
   *
   * @param[in] p   Point in 3-D space
   */
  unsigned int getAxialSectionIndex(const Point & p) const;

  /*
   * Gets the axial element index for a given element center point in 3-D space.
   *
   * @param[in] p_center   Element center point in 3-D space
   */
  unsigned int getAxialElementIndex(const Point & p_center) const;

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

  /**
   * Gets an axis MooseEnum for the axis the component is aligned with.
   *
   * If the axis does not align with the x, y, or z axis, then an invalid
   * MooseEnum is returned.
   */
  MooseEnum getAlignmentAxis() const;

  /**
   * Gets the element boundary coordinates for the aligned axis.
   */
  std::vector<Real> getElementBoundaryCoordinates() const;

protected:
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
  /// Axial coordinate of the end of each axial section using the line 'position' as the origin
  std::vector<Real> _section_end;

  /// Center axial coordinate of each axial element
  std::vector<Real> _x_centers;

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

private:
  /**
   * Computes a normalized direction vector or reports an error if the zero vector is provided
   *
   * @param[in] dir_unnormalized   Unnormalized direction vector
   */
  static RealVectorValue initializeDirectionVector(const RealVectorValue & dir_unnormalized);

public:
  static InputParameters validParams();

  /**
   * Computes the direction transformation tensor
   *
   * @param[in] dir   Direction vector
   */
  static RealTensorValue computeDirectionTransformationTensor(const RealVectorValue & dir);

  /**
   * Computes the rotation transformation tensor
   *
   * @param[in] rotation   Rotation about the x-axis, in degrees
   */
  static RealTensorValue computeXRotationTransformationTensor(const Real & rotation);

  /**
   * Computes point in 3-D space from a point in reference space.
   *
   * @param[in] p   Point in reference space
   */
  static Point computeRealPointFromReferencePoint(const Point & p,
                                                  const RealVectorValue & position,
                                                  const RealTensorValue & R,
                                                  const RealTensorValue & Rx);

  /**
   * Computes point in reference space from a point in 3-D space.
   *
   * @param[in] p   Point in 3-D space
   */
  static Point computeReferencePointFromRealPoint(const Point & p,
                                                  const RealVectorValue & position,
                                                  const RealTensorValue & R_inv,
                                                  const RealTensorValue & Rx_inv);

  /**
   * Gets an axis MooseEnum for the axis the component is aligned with.
   *
   * If the axis does not align with the x, y, or z axis, then an invalid
   * MooseEnum is returned.
   */
  static MooseEnum getAlignmentAxis(const RealVectorValue & dir);

  /**
   * Gets the element boundary coordinates for the aligned axis.
   *
   * @param[in] position     Start position of axis in 3-D space
   * @param[in] orientation  Direction of axis from start position to end position
   * @param[in] rotation     Angle of rotation about the x-axis, in degrees
   * @param[in] lengths      Length of each axial section
   * @param[in] n_elems      Number of elements in each axial section
   */
  static std::vector<Real> getElementBoundaryCoordinates(const RealVectorValue & position,
                                                         const RealVectorValue & orientation,
                                                         const Real & rotation,
                                                         const std::vector<Real> & lengths,
                                                         const std::vector<unsigned int> & n_elems);
};
