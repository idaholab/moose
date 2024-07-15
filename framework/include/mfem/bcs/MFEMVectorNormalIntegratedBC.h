#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.h"

class MFEMVectorNormalIntegratedBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMVectorNormalIntegratedBC(const InputParameters & parameters);
  ~MFEMVectorNormalIntegratedBC() override {}

protected:
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
