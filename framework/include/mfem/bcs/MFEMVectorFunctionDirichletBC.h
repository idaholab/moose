#ifdef MFEM_ENABLED

#pragma once
#include "MFEMVectorFunctionDirichletBCBase.h"

class MFEMVectorFunctionDirichletBC : public MFEMVectorFunctionDirichletBCBase
{

public:
  MFEMVectorFunctionDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctionDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};

#endif
