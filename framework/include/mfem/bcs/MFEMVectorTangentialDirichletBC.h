#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorTangentialDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorTangentialDirichletBC(const InputParameters & parameters);
  ~MFEMVectorTangentialDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
