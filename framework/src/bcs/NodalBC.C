#include "NodalBC.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<NodalBC>()
{
  return validParams<BoundaryCondition>();
}


NodalBC::NodalBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    _current_node(_var.node()),
    _u(_var.nodalSln())
{
}

NodalBC::~NodalBC()
{
}

void
NodalBC::computeResidual(NumericVector<Number> & residual)
{
  unsigned int & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  residual.set(dof_idx, computeQpResidual());
}

void
NodalBC::computeJacobian(SparseMatrix<Number> & jacobian)
{
  // zero the row and put 1.0 on the diagonal
  std::vector<int> zero_rows(1);
  zero_rows[0] = _var.nodalDofIndex();
  jacobian.zero_rows(zero_rows, 1.0);
}

unsigned int
NodalBC::coupled(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupled(var_name, comp);
}

VariableValue &
NodalBC::coupledValue(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledNodalValue(var_name, comp);
}
