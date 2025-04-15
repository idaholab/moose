#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorFunctionDirichletBCBase.h"

class MFEMVectorFunctionTangentialDirichletBC : public MFEMVectorFunctionDirichletBCBase
{
public:
  MFEMVectorFunctionTangentialDirichletBC(const InputParameters & parameters);
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
