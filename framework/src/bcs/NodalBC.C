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
#include "Assembly.h"

template<>
InputParameters validParams<NodalBC>()
{
  InputParameters params = validParams<NodalBCBase>();

  return params;
}


NodalBC::NodalBC(const InputParameters & parameters) :
    NodalBCBase(parameters),
    _moose_var(dynamic_cast<MooseVariable &>(*_variable)),
    _current_node(_moose_var.node()),
    _u(_moose_var.nodalSln()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName> >("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName> >("diag_save_in"))
{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i=0; i<_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving residual values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i=0; i<_diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving diagonal Jacobian values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

void
NodalBC::computeResidual(NumericVector<Number> & residual)
{
  if (_moose_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _moose_var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    residual.set(dof_idx, res);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_save_in.size(); i++)
        _save_in[i]->sys().solution().set(_save_in[i]->nodalDofIndex(), res);
    }
  }
}

void
NodalBC::computeJacobian()
{
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_moose_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _moose_var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_row, cached_val);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().set(_diag_save_in[i]->nodalDofIndex(), cached_val);
    }
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
    dof_id_type cached_row = _moose_var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_col, cached_val);
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
