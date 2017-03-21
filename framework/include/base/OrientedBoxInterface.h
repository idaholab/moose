/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ORIENTEDBOXINTERFACE_H
#define ORIENTEDBOXINTERFACE_H

// MOOSE includes
#include "InputParameters.h"
#include "MooseTypes.h"

// libMesh includes
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

// Forward declarations
class OrientedBoxInterface;

template <>
InputParameters validParams<OrientedBoxInterface>();

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
  OrientedBoxInterface(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~OrientedBoxInterface();

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
  RealTensorValue * _rot_matrix;

  /// The bounding box used to test if the point is contained within
  MeshTools::BoundingBox * _bounding_box;
};

#endif // ORIENTEDBOXINTERFACE_H
