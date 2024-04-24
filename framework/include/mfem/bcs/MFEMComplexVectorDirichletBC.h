#pragma once

#include "MFEMBoundaryCondition.h"
#include "MFEMVectorFunctionCoefficient.h"
#include "boundary_conditions.hpp"

class MFEMComplexVectorDirichletBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMComplexVectorDirichletBC(const InputParameters & parameters);
  ~MFEMComplexVectorDirichletBC() override {}

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  MFEMVectorCoefficient * _vec_coef_re{nullptr};
  MFEMVectorCoefficient * _vec_coef_im{nullptr};
};
