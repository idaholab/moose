#ifndef NODALBC_H
#define NODALBC_H

#include "BoundaryCondition.h"
#include "MooseVariable.h"

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

  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);

protected:
  const Node * & _current_node;

  unsigned int _qp;
  VariableValue & _u;

  virtual Real computeQpResidual() = 0;
};

template<>
InputParameters validParams<NodalBC>();

#endif /* NODALBC_H_ */
