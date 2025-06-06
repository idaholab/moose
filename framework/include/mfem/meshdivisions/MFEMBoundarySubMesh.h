#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSubMeshBase.h"

/**
 * Constructs and stores an mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
 */
class MFEMBoundarySubMesh : public MFEMSubMeshBase
{
public:
  static InputParameters validParams();
  MFEMBoundarySubMesh(const InputParameters & parameters);
  mfem::Array<int> & getBoundaries() { return _bdr_attributes; }

protected:
  virtual void buildSubMesh() override;

  const std::vector<BoundaryName> & _boundary_names;
  mfem::Array<int> _bdr_attributes;
};

#endif
