#ifndef LINEARVECTORPENALTYDIRICHLETBC_H
#define LINEARVECTORPENALTYDIRICHLETBC_H

#include "VectorIntegratedBC.h"

class LinearVectorPenaltyDirichletBC;

template <>
InputParameters validParams<LinearVectorPenaltyDirichletBC>();

class LinearVectorPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  LinearVectorPenaltyDirichletBC(const InputParameters & parameters);

protected:
  Real _penalty;
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Function & _exact_x;
  Function & _exact_y;
};

#endif // LINEARVECTORPENALTYDIRICHLETBC_H
