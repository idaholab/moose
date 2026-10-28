//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

// Forward declarations
namespace libMesh
{
class Elem;
}

// libMesh includes
#include "libmesh/libmesh_common.h" // not possible to forward declare typedef Real.

// C++ includes
#include <array>
#include <utility>
#include <vector>

enum class MortarSegmentTriangulationMode : unsigned char
{
  Delaunay,
  Centroid,
  EarClipping,
  Vertex
};

CreateMooseEnumClass(Mortar3DQuadraturePointMapping,
                     NORMAL_PROJECTION = 0,
                     REFERENCE_INTERPOLATION = 1);

/**
 * Holds xi^(1), xi^(2), and other data for a given mortar segment.
 */
struct MortarSegmentInfo
{
  /**
   * Constructor. The invalid_xi value means that the values have
   * not been set. Valid values are in [-1,1]. xi1 values should
   * always be valid for any segment, but one or both xi2 values
   * can be uninitialized when the surfaces are not in contact.
   */
  MortarSegmentInfo();

  /**
   * Construct a MortarSegmentInfo object with the given xi values.
   */
  MortarSegmentInfo(Real x1a, Real x1b, Real x2a, Real x2b);

  // MortarSegmentInfo(const MortarSegmentInfo &) = default;

  /**
   * Prints xi values and secondary/primary Elem ids.
   */
  void print() const;

  /**
   * Returns true if the current segment is valid, false
   * otherwise. The segment can be "invalid" for a host of different
   * reasons, see the list below.
   */
  bool isValid() const;

  Real xi1_a, xi1_b;
  Real xi2_a, xi2_b;
  const Elem * secondary_elem;
  const Elem * primary_elem;

  // A magic number to let us determine when xi values have not been
  // initialized yet.
  static const Real invalid_xi;
};

/**
 * How a 3D mortar segment node depends on the nodes of the secondary and primary sub-elements
 * whose projected overlap polygon it belongs to. All points are projected onto the secondary
 * subpatch plane.
 */
struct MortarSegmentNodeOrigin
{
  enum class Type : unsigned char
  {
    /// A secondary sub-element node: indices[0] is its sub-element node index
    SecondaryNode,
    /// A primary sub-element node: indices[0] is its sub-element node index
    PrimaryNode,
    /// The intersection of the secondary edge (indices[0], indices[1]) and the primary edge
    /// (indices[2], indices[3]), given by sub-element node indices
    EdgeIntersection,
    /// The arithmetic mean of the polygon nodes indices[0], ..., indices[0] + indices[1] - 1
    MeanCenter,
    /// The area-weighted centroid of the polygon nodes indices[0], ..., indices[0] + indices[1] - 1
    AreaCenter,
    /// A node of unknown origin, whose displacement dependence is neglected
    Fixed
  };

  Type type;
  std::array<unsigned int, 4> indices;
};

/**
 * The origins of the nodes of the overlap polygon that a 3D mortar segment was triangulated from,
 * and which of those nodes are the mortar segment nodes
 */
struct MortarSegmentNodeOrigins
{
  std::vector<MortarSegmentNodeOrigin> polygon;
  std::array<unsigned int, 3> nodes;
};
