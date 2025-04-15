#ifdef MFEM_ENABLED

#pragma once
#include "MFEMFESpace.h"

/// Class with common parameters for MFEMVectorFESpace and
/// MFEMScalarFESpace.
class MFEMSimplifiedFESpace : public MFEMFESpace
{
public:
  static InputParameters validParams();

  MFEMSimplifiedFESpace(const InputParameters & parameters);

protected:
  /// Order of the basis functions in the finite element collection
  const int _fec_order;

  /// Returns the dimension of the problem (i.e., the highest dimension of the
  /// reference elements in the mesh).
  int getProblemDim() const;
};

#endif
