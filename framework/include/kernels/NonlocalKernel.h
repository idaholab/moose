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

#ifndef NONLOCALKERNEL_H
#define NONLOCALKERNEL_H

#include "Kernel.h"

class NonlocalKernel;

template <>
InputParameters validParams<NonlocalKernel>();

/**
 * NonlocalKernel is used for solving integral terms in integro-differential equations.
 * Integro-differential equations includes spatial integral terms over variables in the domain.
 * In this case the jacobian calculation is not restricted to local dofs of an element, it requires
 * additional contributions from all the dofs in the domain. NonlocalKernel adds capability to
 * consider
 * nonlocal jacobians in addition to the local jacobians.
 */
class NonlocalKernel : public Kernel
{
public:
  NonlocalKernel(const InputParameters & parameters);

  /**
   * computeJacobian and computeQpOffDiagJacobian methods are almost same
   * as Kernel except for few additional optimization options regarding the integral terms.
   * Looping order of _i & _j are reversed for providing opimization options and
   * additional getUserObjectJacobian method is provided as an option to obtain
   * jocobians of the integral term from userobject once per dof
   */
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  /**
   * computeNonlocalJacobian and computeNonlocalOffDiagJacobian methods are
   * introduced for providing the jacobian contribution corresponding to nolocal dofs.
   * Nonlocal jacobian block is sized with respect to the all the dofs of jacobian variable.
   * Looping order of _i & _k are reversed for opimization purpose and
   * additional globalDoFEnabled method is provided as an option to execute nonlocal
   * jocobian calculations only for nonlocal dofs that has nonzero jacobian contribution.
   */
  virtual void computeNonlocalJacobian();
  virtual void computeNonlocalOffDiagJacobian(unsigned int jvar);

protected:
  /// Compute this Kernel's contribution to the Jacobian corresponding to nolocal dof at the current quadrature point
  virtual Real computeQpNonlocalJacobian(dof_id_type /*dof_index*/) { return 0; }
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int /*jvar*/, dof_id_type /*dof_index*/)
  {
    return 0;
  }

  /// Optimization option for getting jocobinas from userobject once per dof
  virtual void getUserObjectJacobian(unsigned int /*jvar*/, dof_id_type /*dof_index*/) {}
  /// optimization option for executing nonlocal jacobian calculation only for nonzero elements
  virtual bool globalDoFEnabled(MooseVariable & /*var*/, dof_id_type /*dof_index*/) { return true; }

  unsigned int _k;
};

#endif /* NONLOCALKERNEL_H */
