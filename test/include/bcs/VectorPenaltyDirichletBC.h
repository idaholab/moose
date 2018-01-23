#ifndef VECTORPENALTYDIRICHLETBC_H
#define VECTORPENALTYDIRICHLETBC_H

#include "VectorIntegratedBC.h"

class VectorPenaltyDirichletBC;

template <>
InputParameters validParams<VectorPenaltyDirichletBC>();

class VectorPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  VectorPenaltyDirichletBC(const InputParameters & parameters);

protected:
  Real _penalty;
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Function & _exact_x;
  Function & _exact_y;
  Function & _exact_z;
};

#endif // VECTORPENALTYDIRICHLETBC_H
