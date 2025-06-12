#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorNormalDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorNormalDirichletBC(const InputParameters & parameters);
  ~MFEMVectorNormalDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc) override;
};

#endif
