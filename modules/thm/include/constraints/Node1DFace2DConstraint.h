#pragma once

#include "NodeFaceConstraint.h"

class Node1DFace2DConstraint;

template <>
InputParameters validParams<Node1DFace2DConstraint>();

/**
 * Base class for constraints between 1D flow channel and 2D heat structures
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
