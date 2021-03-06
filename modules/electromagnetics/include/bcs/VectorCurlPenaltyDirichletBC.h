#pragma once

#include "VectorIntegratedBC.h"

class VectorCurlPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorCurlPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  Real _penalty;
  const Function & _exact_x;
  const Function & _exact_y;
  const Function & _exact_z;
};
