#ifndef NODE1DFACE2DCONSTRAINT_H
#define NODE1DFACE2DCONSTRAINT_H

#include "NodeFaceConstraint.h"

class Node1DFace2DConstraint;

template <>
InputParameters validParams<Node1DFace2DConstraint>();

/**
 * Base class for constraints between 1D pipe and 2D heat structures
 */
class Node1DFace2DConstraint : public NodeFaceConstraint
{
public:
  Node1DFace2DConstraint(const InputParameters & parameters);

  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  virtual Real computeQpSlaveValue();
  virtual bool overwriteSlaveResidual();

protected:
  const MooseArray<Real> & _JxW;

  DenseMatrix<Number> _Jee;
  DenseMatrix<Number> _Jen;
  DenseMatrix<Number> _Jnn;
  DenseMatrix<Number> _Jne;
};

#endif /* NODE1DFACE2DCONSTRAINT_H */
