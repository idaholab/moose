//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernelBase.h"

/**
 * Base class for creating nodal kernels with hand-coded Jacobians
 */
class NodalKernel : public NodalKernelBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  NodalKernel(const InputParameters & parameters);

  /**
   * Compute the residual at the current node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpResidual()
   */
  virtual void computeResidual() override;

  /**
   * Compute the Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpJacobian()
   */
  virtual void computeJacobian() override;

  /**
   * Compute the off-diagonal Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpOffDiagJacobian()
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * The user can override this function to compute the residual at a node.
   */
  virtual Real computeQpResidual() = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution.  If not overriden,
   * returns 1.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Value of the unknown variable this is acting on
  const VariableValue & _u;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;
};
