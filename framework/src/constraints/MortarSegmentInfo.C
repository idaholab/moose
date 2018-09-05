#include "MortarSegmentInfo.h"

// libMesh includes
#include "libmesh/elem.h"

using namespace libMesh;

// Initialize constant static members.
const Real MortarSegmentInfo::invalid_xi = 99;

MortarSegmentInfo::MortarSegmentInfo()
  : xi1_a(invalid_xi),
    xi1_b(invalid_xi),
    xi2_a(invalid_xi),
    xi2_b(invalid_xi),
    slave_elem(libmesh_nullptr),
    master_elem(libmesh_nullptr)
{
}

MortarSegmentInfo::MortarSegmentInfo(Real x1a, Real x1b, Real x2a, Real x2b)
  : xi1_a(x1a),
    xi1_b(x1b),
    xi2_a(x2a),
    xi2_b(x2b),
    slave_elem(libmesh_nullptr),
    master_elem(libmesh_nullptr)
{
}

void
MortarSegmentInfo::print() const
{
  libMesh::out << "xi^(1)_a=" << xi1_a << ", xi^(1)_b=" << xi1_b << std::endl;
  libMesh::out << "xi^(2)_a=" << xi2_a << ", xi^(2)_b=" << xi2_b << std::endl;
  if (slave_elem)
    libMesh::out << "slave_elem=" << slave_elem->id() << std::endl;
  if (master_elem)
    libMesh::out << "master_elem=" << master_elem->id() << std::endl;
}

bool
MortarSegmentInfo::is_valid() const
{
  bool b1 = (std::abs(xi1_a) < 1. + TOLERANCE) && (std::abs(xi1_b) < 1. + TOLERANCE);
  bool b2 = (std::abs(xi2_a) < 1. + TOLERANCE) && (std::abs(xi2_b) < 1. + TOLERANCE);

  bool xi2a_unset = (std::abs(xi2_a - invalid_xi) < TOLERANCE);
  bool xi2b_unset = (std::abs(xi2_b - invalid_xi) < TOLERANCE);

  bool xi2_set = !xi2a_unset && !xi2b_unset;

  // Both xi^(1) values must be set to have a valid segment.
  if (!b1)
  {
    libMesh::err << "xi1_a = " << xi1_a << ", xi1_b = " << xi1_b
                 << ", one or both xi^(1) values were not set." << std::endl;
    return false;
  }

  // We don't allow really short segments (this probably means
  // something got screwed up and both xi^(1) values got the same
  // value).
  if (std::abs(xi1_a - xi1_b) < TOLERANCE)
  {
    libMesh::err << "xi^(1) values too close together." << std::endl;
    return false;
  }

  // Must have a valid slave Elem to have a valid segment.
  if (slave_elem == libmesh_nullptr)
  {
    libMesh::err << "Slave Elem was not set." << std::endl;
    return false;
  }

  // Either *both* xi^(2) values should be unset or *neither* should be. Anything else is invalid.
  if ((xi2a_unset && !xi2b_unset) || (!xi2a_unset && xi2b_unset))
  {
    libMesh::err << "One xi^(2) value was set, the other was not set." << std::endl;
    return false;
  }

  // If both xi^(2) values are unset, then master_elem should be NULL.
  if (!xi2_set && master_elem != libmesh_nullptr)
  {
    libMesh::err << "Both xi^(2) are unset, therefore master_elem should be NULL." << std::endl;
    return false;
  }

  // On the other hand, if both xi^(2) values are unset, then make sure master_elem is non-NULL.
  if (xi2_set && master_elem == libmesh_nullptr)
  {
    libMesh::err << "Both xi^(2) are set, the master_elem cannot be NULL." << std::endl;
    return false;
  }

  // If the xi^(2) values are valid, make sure they don't correspond
  // to a really short segment, which probably means they got
  // assigned the same value by accident.
  if (xi2_set && (std::abs(xi2_a - xi2_b) < TOLERANCE))
  {
    libMesh::err << "xi^(2) are too close together." << std::endl;
    return false;
  }

  // If both xi^(2) values are set, they should be in the range.
  if (xi2_set && !b2)
  {
    libMesh::err << "xi^(2) are set, but they are not in the range [-1,1]." << std::endl;
    return false;
  }

  // If we made it here, we're valid.
  return true;
}

bool
MortarSegmentInfo::has_master() const
{
  bool xi2_set =
      (std::abs(xi2_a - invalid_xi) >= TOLERANCE) && (std::abs(xi2_b - invalid_xi) >= TOLERANCE);

  if (xi2_set && master_elem)
    return true;

  return false;
}
