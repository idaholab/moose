//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "ResidualObject.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseVariableInterface.h"

// Forward declerations
class NodalKernel;

template <>
InputParameters validParams<NodalKernel>();

/**
 * Base class for creating new types of boundary conditions
 *
 */
class NodalKernel : public ResidualObject,
                    public BlockRestrictable,
                    public BoundaryRestrictable,
                    public GeometricSearchInterface,
                    public CoupleableMooseVariableDependencyIntermediateInterface,
                    public MooseVariableInterface<Real>
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  NodalKernel(const InputParameters & parameters);

  /**
   * Gets the variable this is active on
   * @return the variable
   */
  const MooseVariable & variable() const override { return _var; }

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

  /// Reference to FEProblemBase
  FEProblemBase & _fe_problem;

  /// variable this works on
  MooseVariable & _var;

  /// current node being processed
  const Node * const & _current_node;

  /// Quadrature point index
  unsigned int _qp;

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
