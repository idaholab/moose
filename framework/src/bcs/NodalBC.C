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
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<RandomInterface>();

  return params;
}


NodalBC::NodalBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    RandomInterface(parameters, _fe_problem, _tid, true),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
    _current_node(_var.node()),
    _u(_var.nodalSln())
{
}

void
NodalBC::computeResidual(NumericVector<Number> & residual)
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    residual.set(dof_idx, computeQpResidual());
  }
}

void
NodalBC::computeJacobian()
{
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheNodalBCJacobianEntry(cached_row, cached_row, cached_val);
  }
}

void
NodalBC::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    _qp = 0;
    Real cached_val = computeQpOffDiagJacobian(jvar);
    dof_id_type cached_row = _var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheNodalBCJacobianEntry(cached_row, cached_col, cached_val);
  }
}


Real
NodalBC::computeQpJacobian()
{
  return 1.;
}

Real
NodalBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
