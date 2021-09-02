//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarSegmentInfo.h"
#include "MooseError.h"

#include "libmesh/elem.h"

using namespace libMesh;

// Initialize constant static members.
const Real MortarSegmentInfo::invalid_xi = 99;

MortarSegmentInfo::MortarSegmentInfo()
  : xi1_a(invalid_xi),
    xi1_b(invalid_xi),
    xi2_a(invalid_xi),
    xi2_b(invalid_xi),
    secondary_elem(nullptr),
    primary_elem(nullptr)
{
}

MortarSegmentInfo::MortarSegmentInfo(Real x1a, Real x1b, Real x2a, Real x2b)
  : xi1_a(x1a), xi1_b(x1b), xi2_a(x2a), xi2_b(x2b), secondary_elem(nullptr), primary_elem(nullptr)
{
}

void
MortarSegmentInfo::print() const
{
  Moose::out << "xi^(1)_a=" << xi1_a << ", xi^(1)_b=" << xi1_b << std::endl;
  Moose::out << "xi^(2)_a=" << xi2_a << ", xi^(2)_b=" << xi2_b << std::endl;
  if (secondary_elem)
    Moose::out << "secondary_elem=" << secondary_elem->id() << std::endl;
  if (primary_elem)
    Moose::out << "primary_elem=" << primary_elem->id() << std::endl;
}

bool
MortarSegmentInfo::isValid() const
{
  bool b1 = (std::abs(xi1_a) < 1. + TOLERANCE) && (std::abs(xi1_b) < 1. + TOLERANCE);
  bool b2 = (std::abs(xi2_a) < 1. + TOLERANCE) && (std::abs(xi2_b) < 1. + TOLERANCE);

  bool xi2a_unset = (std::abs(xi2_a - invalid_xi) < TOLERANCE);
  bool xi2b_unset = (std::abs(xi2_b - invalid_xi) < TOLERANCE);

  bool xi2_set = !xi2a_unset && !xi2b_unset;

  // Both xi^(1) values must be set to have a valid segment.
  if (!b1)
  {
    mooseError("xi1_a = ", xi1_a, ", xi1_b = ", xi1_b, ", one or both xi^(1) values were not set.");
    return false;
  }

  // We don't allow really short segments (this probably means
  // something got screwed up and both xi^(1) values got the same
  // value).
  if (std::abs(xi1_a - xi1_b) < TOLERANCE)
  {
    mooseError("xi^(1) values too close together.");
    return false;
  }

  // Must have a valid secondary Elem to have a valid segment.
  if (secondary_elem == nullptr)
  {
    mooseError("Secondary Elem was not set.");
    return false;
  }

  // Either *both* xi^(2) values should be unset or *neither* should be. Anything else is invalid.
  if ((xi2a_unset && !xi2b_unset) || (!xi2a_unset && xi2b_unset))
  {
    mooseError("One xi^(2) value was set, the other was not set.");
    return false;
  }

  // If both xi^(2) values are unset, then primary_elem should be NULL.
  if (!xi2_set && primary_elem != nullptr)
  {
    mooseError("Both xi^(2) are unset, therefore primary_elem should be NULL.");
    return false;
  }

  // On the other hand, if both xi^(2) values are unset, then make sure primary_elem is non-NULL.
  if (xi2_set && primary_elem == nullptr)
  {
    mooseError("Both xi^(2) are set, the primary_elem cannot be NULL.");
    return false;
  }

  // If the xi^(2) values are valid, make sure they don't correspond
  // to a really short segment, which probably means they got
  // assigned the same value by accident.
  if (xi2_set && (std::abs(xi2_a - xi2_b) < TOLERANCE))
  {
    mooseError("xi^(2) are too close together.");
    return false;
  }

  // If both xi^(2) values are set, they should be in the range.
  if (xi2_set && !b2)
  {
    mooseError("xi^(2) are set, but they are not in the range [-1,1].");
    return false;
  }

  // If we made it here, we're valid.
  return true;
}
