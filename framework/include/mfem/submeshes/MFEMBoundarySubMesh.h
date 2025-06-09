#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSubMesh.h"

/**
 * Constructs and stores an mfem::ParSubMesh object as
 * as a restriction of the parent mesh to the set of user-specified boundaries.
 * Access using the getSubMesh() accessor.
 */
class MFEMBoundarySubMesh : public MFEMSubMesh
{
public:
  static InputParameters validParams();
  MFEMBoundarySubMesh(const InputParameters & parameters);
  const mfem::Array<int> & getBoundaries() { return _bdr_attributes; }

protected:
  virtual void buildSubMesh() override;

  const std::vector<BoundaryName> & _boundary_names;
  mfem::Array<int> _bdr_attributes;
};

#endif
