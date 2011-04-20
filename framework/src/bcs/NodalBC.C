/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
