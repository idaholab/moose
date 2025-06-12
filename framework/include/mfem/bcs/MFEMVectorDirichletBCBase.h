#ifdef MFEM_ENABLED

#pragma once

#include "MFEMEssentialBC.h"

class MFEMVectorDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMVectorDirichletBCBase() override = default;

protected:
  MFEMVectorDirichletBCBase(const InputParameters & parameters);
  const MFEMVectorCoefficientName & _vec_coef_name;
  mfem::VectorCoefficient & _vec_coef;
};

#endif
