#ifndef NODALBC_H_
#define NODALBC_H_

#include "BoundaryCondition.h"
#include "Variable.h"

// libMesh
#include "numeric_vector.h"
#include "sparse_matrix.h"

class NodalBC : public BoundaryCondition
{
public:
  NodalBC(const std::string & name, InputParameters parameters);
  virtual ~NodalBC();

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  const Node * & _node;

  unsigned int _qp;
  VariableValue & _u;

  virtual Real computeNodeResidual() = 0;
};

template<>
InputParameters validParams<NodalBC>();

#endif /* NODALBC_H_ */
