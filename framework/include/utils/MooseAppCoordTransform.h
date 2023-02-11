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
#include "Units.h"
#include "libmesh/point.h"
#include "libmesh/tensor_value.h"
#include <memory>
#include <string>
#include <array>
#include <tuple>

class InputParameters;
class MooseMesh;

class MooseAppCoordTransform
{
public:
  /**
   * A class scope enumeration for conveniently denoting X, Y, and Z axis directions
   */
  enum Direction : unsigned int
  {
    X = 0,
    Y,
    Z,
    INVALID
  };

  /**
   * A typedef for conveniency that describes the minimal data necessary to broadcast and build a \p
   * MooseAppCoordTransform. The data is such:
   * 0: whether a scaling matrix exists
   * 1: a value describing the scaling
   * 2: whether a rotation matrix exists
   * 3: the Euler angles describing the rotation
   * 4: the coordinate system type
   * 5: the r-axis direction
   * 6: the z-axis direction
   * 7: whether there are multiple coordinate system types on the mesh
   * 8: whether the mesh has been transformed using the transform
   */
  typedef std::tuple<short int,
                     Real,
                     short int,
                     std::array<Real, 3>,
                     int,
                     unsigned int,
                     unsigned int,
                     short int,
                     short int>
      MinimalData;

  /**
   * Default constructor. If no other methods are called to set rotation, translation, or scaling,
   * then when \p operator() is called the result will be the passed-in point, e.g. no
   * transformation will occur
   */
  MooseAppCoordTransform();

  MooseAppCoordTransform(const MooseAppCoordTransform & other); // we have unique pointers
  MooseAppCoordTransform(MooseAppCoordTransform && other) = default;

  /**
   * Construct a coordinate transformation object from the minimal set of data required
   */
  MooseAppCoordTransform(const MinimalData & minimal_data);

  /**
   * Construct this object from the provided mesh and its input parameters. See the \p validParams
   * implementation for valid parameters
   */
  MooseAppCoordTransform(const MooseMesh & mesh);

  ~MooseAppCoordTransform() = default;

  MooseAppCoordTransform &
  operator=(const MooseAppCoordTransform & other); // we have unique pointers
  MooseAppCoordTransform & operator=(MooseAppCoordTransform && other) = default;

  /**
   * Describes the parameters this object can take to setup transformations. These include
   * parameters related to coordinate system type, rotation, and scaling
   */
  static InputParameters validParams();

  /**
   * @return the minimal data necessary to describe this coordinate transformation object. This data
   * can be broadcast and used to construct identical coordinate transformation objects on other
   * processes
   */
  MinimalData minimalDataDescription() const;

  /**
   * @return our coordinate system
   */
  Moose::CoordinateSystemType coordinateSystem() const { return _coord_type; }

  /**
   * Set how much our domain should be translated in order to match a reference frame. In practice
   * we choose the parent application to be the reference frame with respect to translation, e.g.
   * the parent application origin is the reference frame origin, and we set the translation vectors
   * of child applications to the multiapp positions parameter. Similarly to the \p setRotation with
   * angles API, this represents a forward transformation from our domain to the reference domain
   */
  void setTranslationVector(const libMesh::Point & translation);

  /**
   * Will setup a rotation transformation. The rotation transformation will be a single 90-degree
   * rotation defined such that a point on the axis specified by \p up_direction is rotated onto the
   * Y-axis, which is our canonical/reference-frame up-direction
   * @param up_direction What direction corresponds to "up" (e.g. the opposite direction of gravity)
   * in our moose mesh
   */
  void setUpDirection(Direction up_direction);

  /**
   * Setup an \emph extrinsic rotation defined in the following way:
   * 1. rotate by \p alpha degrees about the z-axis
   * 2. rotate by \p beta degrees about the x-axis
   * 3. rotate by \p gamma degrees about the z-axis
   * Definitions of the resulting matrix are found in the last row of the Proper Euler angles column
   * of https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix. These rotation angles should
   * describe how points in our domain should be rotated in order to arrive back in the reference
   * frame. For instance, in 2D your mesh may appear 90 degrees rotated (around the z-axis) with
   * respect to the reference frame. In such a case, the angle set you should provide to this
   * function is {-90, 0, 0}, e.g. provide forward transformation angles that will map points from
   * your domain to the reference domain
   *
   * If our coordinate system is RZ, then only certain values of alpha, beta, and gamma will be
   * accepted such that the radial and axial coordinates are rotated onto Cartesian axes and the
   * resulting radial coordinate is non-negative
   */
  void setRotation(Real alpha, Real beta, Real gamma);

  /**
   * Set the scaling transformation
   * @param length_unit How much distance one mesh length unit represents, e.g. 1 cm, 1 nm, 1 ft, 5
   * inches. We will save off the value provided to this in the \p _length_unit data member as well
   * as set the scaling transform
   */
  void setLengthUnit(const MooseUnits & length_unit);

  /**
   * @return How much distance one mesh length unit represents, e.g. 1 cm, 1 nm, 1 ft, 5
   * inches
   */
  const MooseUnits & lengthUnit() const { return _length_unit; }

  /**
   * Set our coordinate system
   * @param system_type the coordinate system type
   * @param rz_symmetry_axis the axial coordinate, e.g. the axis of symmetry
   */
  void setCoordinateSystem(Moose::CoordinateSystemType system_type,
                           Direction rz_symmetry_axis = INVALID);

  /**
   * Set our coordinate system based on the MooseMesh coordinate system data
   */
  void setCoordinateSystem(const MooseMesh & mesh);

  /**
   * Compute the RS and (RS)^{-1} matrices
   */
  void computeRS();

  /**
   * Transforms the entire mesh with the coordinate transform
   * This can be done to output in position, or to avoid transforming on every data point
   * @param mesh the mesh to modify, usually the child app mesh
   * @param translation the translation to apply to the mesh, often the app position
   */
  void transformMesh(MooseMesh & mesh, const libMesh::Point & translation);

private:
  /**
   * If the coordinate system type is RZ, then we return the provided argument. Otherwise we return
   * INVALID
   */
  Direction processZAxis(Direction z_axis);

  /// Represents a forward scaling transformation from our units to reference frame units of meters. This
  /// matrix will be diagonal
  std::unique_ptr<libMesh::RealTensorValue> _scale;

  /// Represents a forward rotation transformation from our domain to the reference frame domain
  std::unique_ptr<libMesh::RealTensorValue> _rotate;

  /// Represents the product of rotation and scaling transformations
  std::unique_ptr<libMesh::RealTensorValue> _rs;

  /// Represents the inverse of the product of rotation and scaling transformations
  std::unique_ptr<libMesh::RealTensorValue> _rs_inverse;

  /// Our coordinate system
  Moose::CoordinateSystemType _coord_type;
  /// If we are RZ or RSPHERICAL, the Cartesian axis corresponding to the radial coordinate
  Direction _r_axis;
  /// If we are RZ, the Cartesian axis corresponding to the axial/axis-of-symmetry coordinate
  Direction _z_axis;

  /// Whether we have different coordinate systems within our single domain. If we do, this will be
  /// problematic if we need to collapse from our space into an RZ or RSPHERICAL space because we
  /// are only ever provided with a point argument and not a subdomain ID argument. Consequently we
  /// will not know in what coordinate system our point lies and will not know how to perform the
  /// dimension collapse, and so we will error
  bool _has_different_coord_sys;

  /// How much distance one mesh length unit represents, e.g. 1 cm, 1 nm, 1 ft, 5 inches
  MooseUnits _length_unit;

  /// The Euler angles describing rotation
  std::array<Real, 3> _euler_angles;

  /// Whether the mesh has been translated and rotated. In this case, applying the transform every
  /// time is no longer necessary
  bool _mesh_transformed;

  friend class MultiAppCoordTransform;
};

/**
 * This class contains transformation information that only exists in a context in which there are
 * multiple applications. Such information includes translation and coordinate collapsing
 */
class MultiAppCoordTransform
{
public:
  explicit MultiAppCoordTransform(const MooseAppCoordTransform & our_app_transform);

  /**
   * Transforms a point from our domain into the reference domain. The sequence of transformations
   * applied is:
   * 1. Scaling
   * 2. Rotation
   * 3. Translation
   * 4. Potential collapse of XYZ coordinates into RZ or RSPHERICAL coordinates depending on the
   *    destination coordinate system (if there is no destination coordinate system or the
   *    destination coordinate system is XYZ, then nothing happens in this stage)
   * @param point A point in our domain
   * @return The corresponding position in the reference domain
   */
  libMesh::Point operator()(const libMesh::Point & point) const;

  /**
   * Inverse transform from the reference space to our space. This will error if coordinate
   * collapsing would occur in \p operator(). When doing inversion we invert the order of
   * operations, e.g. we will perform
   * 1. invert translation
   * 2. invert rotation
   * 3. invert scaling
   */
  libMesh::Point mapBack(const libMesh::Point & point) const;

  /**
   * Set how much our domain should be translated in order to match a reference frame. In practice
   * we choose the parent application to be the reference frame with respect to translation, e.g.
   * the parent application origin is the reference frame origin, and we set the translation vectors
   * of child applications to the multiapp positions parameter. Similarly to the \p setRotation with
   * angles API, this represents a forward transformation from our domain to the reference domain
   */
  void setTranslationVector(const libMesh::Point & translation);

  /**
   * Set the destination coordinate system and destination radial and symmetry axes as appropriate
   * for RZ or RSPHERICAL simulations. Depending on the coordinate system type of the provided
   * coordinate transform we may perform additional transformations. For instance if the destination
   * coordinate system is RZ and we are XYZ, we will translate our xyz points into RZ points, e.g.
   * we will collapse from three dimensions into two. The transformation would be non-unique if we
   * were to attempt to go from RZ to XYZ, e.g. a single RZ point could correspond to any point in a
   * 2pi rotation around the symmetry axis
   */
  void setDestinationCoordTransform(const MooseAppCoordTransform & destination_coord_transform);

  /**
   * @return whether the coordinate transformation object modifies an incoming point, e.g. whether
   * the transformation is anything other than the identity matrix
   */
  bool isIdentity() const;

  /**
   * @return whether there are any transformations other than translation in the transform. We have
   * this method because translation has always been supported natively by the multiapp transfer
   * system through the 'positions' parameter
   */
  bool hasNonTranslationTransformation() const;

  /**
   * @return our coordinate system
   */
  Moose::CoordinateSystemType coordinateSystem() const { return _our_app_transform._coord_type; }

  /**
   * set whether coordinate collapsing operations should be skipped
   */
  void skipCoordinateCollapsing(bool skip_coordinate_collapsing);

  /**
   * whether coordinate collapsing operations should be skipped
   */
  bool skipCoordinateCollapsing() const { return _skip_coordinate_collapsing; }

  /**
   * @return our moose-app coordinate transformation object
   */
  const MooseAppCoordTransform & ourAppTransform() const { return _our_app_transform; }

private:
  /// A reference to the \p MooseAppCoordTransform object that describes scaling, rotation, and
  /// coordinate system transformations from our domain to the reference domain,
  /// e.g. transformations that occur irrespective of the existence of other applications
  const MooseAppCoordTransform & _our_app_transform;

  /// A pointer to the \p MooseAppCoordTransform object that describes scaling, rotation, and
  /// coordinate system transformations from the destination domain to the reference domain,
  /// e.g. transformations that occur irrespective of the existence of other applications
  /// This attribute is currently mostly providing only the coordinate system for conversions
  /// and sanity checking. The actual transformation of destination app points in transfers is done
  /// by the MultiAppCoordTransform for the other direction
  const MooseAppCoordTransform * _destination_app_transform;

  /// Describes a forward translation transformation from our domain to the reference frame domain
  libMesh::Point _translation;

  /// whether coordinate collapsing operations should be skipped
  bool _skip_coordinate_collapsing;
};
