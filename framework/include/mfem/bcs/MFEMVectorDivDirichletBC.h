#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorDivDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  MFEMVectorDivDirichletBC(const InputParameters & parameters);
  ~MFEMVectorDivDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
