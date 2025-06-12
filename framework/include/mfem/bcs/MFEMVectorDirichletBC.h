#ifdef MFEM_ENABLED

#pragma once
#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorDirichletBC : public MFEMVectorDirichletBCBase
{

public:
  MFEMVectorDirichletBC(const InputParameters & parameters);
  ~MFEMVectorDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc) override;
};

#endif
