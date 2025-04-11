#pragma once

#include "MFEMVectorFunctionDirichletBCBase.h"

class MFEMVectorFunctionTangentialDirichletBC : public MFEMVectorFunctionDirichletBCBase
{
public:
  MFEMVectorFunctionTangentialDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctionTangentialDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
