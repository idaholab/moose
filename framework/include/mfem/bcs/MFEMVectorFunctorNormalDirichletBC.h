#pragma once

#include "MFEMVectorFunctorDirichletBCBase.h"

class MFEMVectorFunctorNormalDirichletBC : public MFEMVectorFunctorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorFunctorNormalDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctorNormalDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
