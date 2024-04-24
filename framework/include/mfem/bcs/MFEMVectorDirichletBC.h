#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.hpp"

class MFEMVectorDirichletBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMVectorDirichletBC(const InputParameters & parameters);
  ~MFEMVectorDirichletBC() override {}

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
