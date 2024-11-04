#pragma once

#include "MFEMEssentialBC.h"
#include "boundary_conditions.h"

class MFEMVectorFunctionDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMVectorFunctionDirichletBCBase() override = default;

protected:
  MFEMVectorFunctionDirichletBCBase(const InputParameters & parameters);
  std::shared_ptr<mfem::VectorCoefficient> _vec_coef{nullptr};
};
