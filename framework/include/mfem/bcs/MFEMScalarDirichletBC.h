#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMFunctionCoefficient.h"
#include "boundary_conditions.h"

class MFEMScalarDirichletBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMScalarDirichletBC(const InputParameters & parameters);
  ~MFEMScalarDirichletBC() override {}

protected:
  MFEMCoefficient * _coef{nullptr};
};
