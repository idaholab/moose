#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.h"

class MFEMComplexVectorDirichletBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMComplexVectorDirichletBC(const InputParameters & parameters);
  ~MFEMComplexVectorDirichletBC() override {}

protected:
  MFEMVectorCoefficient * _vec_coef_re{nullptr};
  MFEMVectorCoefficient * _vec_coef_im{nullptr};
};
