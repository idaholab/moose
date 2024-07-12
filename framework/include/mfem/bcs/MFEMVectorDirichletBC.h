#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.h"

class MFEMVectorDirichletBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMVectorDirichletBC(const InputParameters & parameters);
  ~MFEMVectorDirichletBC() override {}

protected:
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
