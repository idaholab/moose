#pragma once

#include "MFEMVectorFunctorDirichletBCBase.h"

class MFEMVectorFunctorTangentialDirichletBC : public MFEMVectorFunctorDirichletBCBase
{
public:
  MFEMVectorFunctorTangentialDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctorTangentialDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
