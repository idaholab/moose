#pragma once

#include "MFEMEssentialBC.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.h"

class MFEMVectorDirichletBCBase : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMVectorDirichletBCBase() override = default;

protected:
  MFEMVectorDirichletBCBase(const InputParameters & parameters);
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
