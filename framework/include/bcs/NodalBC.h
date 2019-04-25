//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBCBase.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseVariableInterface.h"

// Forward declarations
class NodalBC;

template <>
InputParameters validParams<NodalBC>();

/**
 * Base class for deriving any boundary condition that works at nodes
 */
class NodalBC : public NodalBCBase, public MooseVariableInterface<Real>
{
public:
  NodalBC(const InputParameters & parameters);

  /**
   * Gets the variable this BC is active on
   * @return the variable
   */
  virtual MooseVariable & variable() override { return _var; }
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  MooseVariable & _var;

  /// current node being processed
  const Node *& _current_node;

  /// Quadrature point index
  unsigned int _qp;
  /// Value of the unknown variable this BC is acting on
  const VariableValue & _u;

  virtual Real computeQpResidual() = 0;
  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution for this NodalBC.  If not overriden,
   * returns 1.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

