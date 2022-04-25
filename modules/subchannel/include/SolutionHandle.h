#pragma once

#include "SystemBase.h"
#include "libmesh/system.h"
#include "libmesh/enum_norm_type.h"
#include "libmesh/petsc_vector.h"

/**
 * Provide a simple RAII interface for linear lagrange solution variables.
 */
class SolutionHandle
{
public:
  SolutionHandle(const MooseVariableFieldBase & variable)
    : _var(const_cast<MooseVariableFieldBase &>(variable)),
      _soln(_var.sys().solution()),
      _soln_old(_var.sys().solutionOld())
  {
  }

  /**
   * Get a value from the solution vector.
   */
  Number operator()(Node * node)
  {
    // The 0 assumes linear Lagrange (I think)
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    return _soln(dof);
  }
  /**
   * Get a value from the old solution vector.
   */
  Number old(Node * node)
  {
    // The 0 assumes linear Lagrange (I think)
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    return _soln_old(dof);
  }
  /**
   * Set a value in the solution vector.
   */
  void set(Node * node, Number val)
  {
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    _soln.set(dof, val);
  }

  Real L2norm() { return _var.sys().system().calculate_norm(_soln, _var.number(), DISCRETE_L2); }

private:
  MooseVariableFieldBase & _var;
  NumericVector<Number> & _soln;
  NumericVector<Number> & _soln_old;
};
