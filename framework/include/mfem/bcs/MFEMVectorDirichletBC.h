#ifdef MFEM_ENABLED

#pragma once
#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorDirichletBC : public MFEMVectorDirichletBCBase
{

public:
  MFEMVectorDirichletBC(const InputParameters & parameters);
  ~MFEMVectorDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
