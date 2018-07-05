#ifndef VECTORCURLPENALTYDIRICHLETBC_H
#define VECTORCURLPENALTYDIRICHLETBC_H

#include "VectorIntegratedBC.h"

class VectorCurlPenaltyDirichletBC;

template <>
InputParameters validParams<VectorCurlPenaltyDirichletBC>();

/**
 *
 */
class VectorCurlPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  VectorCurlPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Real _penalty;
  Function & _exact_x;
  Function & _exact_y;
  Function & _exact_z;
};

#endif // VECTORCURLPENALTYDIRICHLETBC_H
