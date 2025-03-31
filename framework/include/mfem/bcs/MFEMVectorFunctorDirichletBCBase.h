#pragma once

#include "MFEMEssentialBC.h"
#include "boundary_conditions.h"

class MFEMVectorFunctorDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMVectorFunctorDirichletBCBase() override = default;

protected:
  MFEMVectorFunctorDirichletBCBase(const InputParameters & parameters);
  mfem::VectorCoefficient & _vec_coef;
};
