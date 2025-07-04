//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Base class for construction of a mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
 */
class MFEMSubMesh : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMSubMesh(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed fespace.
  inline std::shared_ptr<mfem::ParSubMesh> getSubMesh()
  {
    if (!_submesh)
      buildSubMesh();
    return _submesh;
  }

protected:
  /// Stores the constructed submesh.
  std::shared_ptr<mfem::ParSubMesh> _submesh{nullptr};
  /// Constructs the submesh.
  virtual void buildSubMesh() = 0;
};

#endif
