#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorTangentialDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorTangentialDirichletBC(const InputParameters & parameters);
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
