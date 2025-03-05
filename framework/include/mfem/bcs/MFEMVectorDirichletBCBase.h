#ifdef MFEM_ENABLED

#pragma once

#include "MFEMEssentialBC.h"

class MFEMVectorDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

protected:
  MFEMVectorDirichletBCBase(const InputParameters & parameters);
  std::vector<Real> _vec_value;
  const std::shared_ptr<mfem::VectorCoefficient> _vec_coef;
};

#endif
