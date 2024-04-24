#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.hpp"

class MFEMVectorNormalIntegratedBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMVectorNormalIntegratedBC(const InputParameters & parameters);
  ~MFEMVectorNormalIntegratedBC() override {}

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
