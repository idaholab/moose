#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorCurlDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  MFEMVectorCurlDirichletBC(const InputParameters & parameters);
  ~MFEMVectorCurlDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;
};
