//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

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
  static InputParameters validParams();

  NonlocalKernel(const InputParameters & parameters);

  /**
   * computeJacobian and computeQpOffDiagJacobian methods are almost same
   * as Kernel except for few additional optimization options regarding the integral terms.
   * Looping order of _i & _j are reversed for providing opimization options and
   * additional getUserObjectJacobian method is provided as an option to obtain
   * jocobians of the integral term from userobject once per dof
   */
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * computeNonlocalJacobian and computeNonlocalOffDiagJacobian methods are
   * introduced for providing the jacobian contribution corresponding to nolocal dofs.
   * Nonlocal jacobian block is sized with respect to the all the dofs of jacobian variable.
   * Looping order of _i & _k are reversed for opimization purpose and
   * additional globalDoFEnabled method is provided as an option to execute nonlocal
   * jocobian calculations only for nonlocal dofs that has nonzero jacobian contribution.
   */
  virtual void computeNonlocalJacobian() override;
  virtual void computeNonlocalOffDiagJacobian(unsigned int jvar) override;

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
  virtual bool globalDoFEnabled(MooseVariableFEBase & /*var*/, dof_id_type /*dof_index*/)
  {
    return true;
  }

  unsigned int _k;
};
