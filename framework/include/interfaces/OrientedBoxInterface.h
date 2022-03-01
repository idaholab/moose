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
#include "MooseTypes.h"

#include "libmesh/bounding_box.h" // For destructor
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

// Forward declarations
class InputParameters;
template <typename T>
InputParameters validParams();

/*
 * An interface class for testing if a point is within a bounding box with arbitrary orientation
 *
 * This constructor does most of the work.
 * The overall strategy is to create a box of the required size which is centered at the origin,
 * with
 * the width along the x axis, the length along the y axis, and the height along the z axis
 *
 * Then create the transformation from real space into this box, which is:
 * a translation from center to the origin, then
 * a rotation from the oriented box frame to this frame
 *
 * see OrientedBoxMarker OrientedSubdomainBoundingBox
 */
class OrientedBoxInterface
{
public:
  /**
   * Class constructor
   */
  static InputParameters validParams();

  OrientedBoxInterface(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~OrientedBoxInterface() = default;

protected:
  /**
   * Test if the supplied point is within the defined oriented bounding box
   * @param point The point to test
   * @return True if the supplied point is within the bounding box
   */
  bool containsPoint(const Point & point);

private:
  /// Center of the defined bounding box
  Point _center;

  /// Rotation matrix for transforming the bounding box
  std::unique_ptr<RealTensorValue> _rot_matrix;

  /// The bounding box used to test if the point is contained within
  std::unique_ptr<BoundingBox> _bounding_box;
};
