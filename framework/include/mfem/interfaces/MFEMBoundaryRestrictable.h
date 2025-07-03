//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "GeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Base class for construction of an object that is restricted to a subset
 * of boundaries of the problem mesh.
 */
class MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMBoundaryRestrictable(const InputParameters & parameters, const mfem::ParMesh & mfem_mesh);

  mfem::Array<int> boundariesToAttributes(const std::vector<BoundaryName> & boundary_names);

  /// Returns a bool indicating if the object is restricted to a subset of boundaries
  bool isBoundaryRestricted() const
  {
    return !(_boundary_attributes.Size() == 1 && _boundary_attributes[0] == -1);
  }

  const mfem::ParMesh & getMesh() { return _mfem_mesh; }
  const mfem::Array<int> & getBoundaryAttributes() { return _boundary_attributes; }
  mfem::Array<int> & getBoundaryMarkers() { return _boundary_markers; }

protected:
  /// Stores the names of the boundaries.
  const mfem::ParMesh & _mfem_mesh;
  /// Stores the names of the boundaries.
  std::vector<BoundaryName> _boundary_names;
  /// Array storing boundary attribute IDs for this object.
  mfem::Array<int> _boundary_attributes;
  /// Boolean array indicating which boundaries are active in this object.
  mfem::Array<int> _boundary_markers;
};

#endif
