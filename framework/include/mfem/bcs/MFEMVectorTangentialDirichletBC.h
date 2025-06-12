#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorTangentialDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  MFEMVectorTangentialDirichletBC(const InputParameters & parameters);
  ~MFEMVectorTangentialDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
