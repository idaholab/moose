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
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Base class for construction of a mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
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

  mfem::Array<int> & getBoundaries() { return _boundary_markers; }
  const mfem::ParMesh & getMesh() { return _mfem_mesh; }

protected:
  /// Stores the names of the boundaries.
  const mfem::ParMesh & _mfem_mesh;
  std::vector<BoundaryName> _boundary_names;
  mfem::Array<int> _boundary_attributes;
  mfem::Array<int> _boundary_markers;
};

#endif
