#pragma once

#include "SystemBase.h"

/**
 * Provide a simple RAII interface for linear lagrange solution variables.
 */
class SolutionHandle
{
public:
  SolutionHandle(const MooseVariableFieldBase & variable) : _var(const_cast<MooseVariableFieldBase &>(variable)), _soln(_var.sys().solution())
  {
  }

  ~SolutionHandle() { _soln.close(); }

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
   * Set a value in the solution vector.
   */
  void set(Node * node, Number val)
  {
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    _soln.set(dof, val);
  }

private:
  MooseVariableFieldBase & _var;
  NumericVector<Number> & _soln;
};
