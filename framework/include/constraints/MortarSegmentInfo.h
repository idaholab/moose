//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Forward declarations
namespace libMesh
{
class Elem;
}

// libMesh includes
#include "libmesh/libmesh_common.h" // not possible to forward declare typedef Real.

// C++ includes
#include <utility>

// Using statements
using libMesh::Elem;
using libMesh::Real;

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

  /**
   * Returns true if this segment has a valid primary, false otherwise.
   */
  bool hasPrimary() const;

  Real xi1_a, xi1_b;
  Real xi2_a, xi2_b;
  const Elem * secondary_elem;
  const Elem * primary_elem;

  // A magic number to let us determine when xi values have not been
  // initialized yet.
  static const Real invalid_xi;
};
