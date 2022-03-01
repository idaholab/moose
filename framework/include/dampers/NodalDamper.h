//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "Damper.h"
#include "MaterialPropertyInterface.h"
#include "MooseTypes.h"

class SubProblem;
class SystemBase;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
class Assembly;

/**
 * Base class for deriving nodal dampers
 */
class NodalDamper : public Damper, protected MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  NodalDamper(const InputParameters & parameters);

  /**
   * Computes this Damper's damping for one node.
   */
  Real computeDamping();

  /**
   * Get the variable this damper is acting on
   */
  MooseVariable * getVariable() { return &_var; }

protected:
  /**
   * This MUST be overridden by a child damper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping() = 0;

  /// Thread ID
  THREAD_ID _tid;
  Assembly & _assembly;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Non-linear variable this damper works on
  MooseVariable & _var;

  /// Current node
  const Node * const & _current_node;

  /// Quadrature point index
  unsigned int _qp;

  /// The current Newton increment
  const VariableValue & _u_increment;
  /// Holds the current solution at the current node
  const VariableValue & _u;
};
