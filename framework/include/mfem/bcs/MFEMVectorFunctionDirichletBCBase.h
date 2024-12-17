#ifdef MFEM_ENABLED

#pragma once

#include "MFEMEssentialBC.h"

class MFEMVectorFunctionDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

protected:
  MFEMVectorFunctionDirichletBCBase(const InputParameters & parameters);
  mfem::VectorCoefficient & _vec_coef;
};

#endif
