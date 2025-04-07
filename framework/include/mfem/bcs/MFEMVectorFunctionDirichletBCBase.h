#ifdef MFEM_ENABLED

#pragma once

#include "MFEMEssentialBC.h"
#include "MFEMBoundaryConditionUtils.h"

class MFEMVectorFunctionDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMVectorFunctionDirichletBCBase() override = default;

protected:
  MFEMVectorFunctionDirichletBCBase(const InputParameters & parameters);
  const std::shared_ptr<mfem::VectorCoefficient> _vec_coef;
};

#endif
