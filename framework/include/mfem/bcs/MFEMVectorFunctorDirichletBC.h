#pragma once
#include "MFEMVectorFunctorDirichletBCBase.h"

class MFEMVectorFunctorDirichletBC : public MFEMVectorFunctorDirichletBCBase
{

public:
  MFEMVectorFunctorDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctorDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
