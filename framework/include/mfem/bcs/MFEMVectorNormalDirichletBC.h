#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorNormalDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  MFEMVectorNormalDirichletBC(const InputParameters & parameters);
  ~MFEMVectorNormalDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
