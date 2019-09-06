#pragma once

#include "VectorIntegratedBC.h"

class VectorCurlPenaltyDirichletBC;

template <>
InputParameters validParams<VectorCurlPenaltyDirichletBC>();

class VectorCurlPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  VectorCurlPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Real _penalty;
  const Function & _exact_x;
  const Function & _exact_y;
  const Function & _exact_z;
};
