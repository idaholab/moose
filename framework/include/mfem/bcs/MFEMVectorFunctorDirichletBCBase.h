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
  const MFEMVectorCoefficientName & _vec_coef_name;
  mfem::VectorCoefficient & _vec_coef;
};
