#include "NodalBC.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<NodalBC>()
{
  return validParams<BoundaryCondition>();
}


NodalBC::NodalBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    Coupleable(parameters, true),
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
  mooseError("This shouldn't be called!");
}
