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

#ifndef NODALDAMPER_H
#define NODALDAMPER_H

// Moose Includes
#include "Damper.h"
#include "ParallelUniqueId.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariableBase.h"

// Forward Declarations
class NodalDamper;
class SubProblem;
class SystemBase;
class MooseVariable;
class Assembly;

template <>
InputParameters validParams<NodalDamper>();

/**
 * Base class for deriving nodal dampers
 */
class NodalDamper : public Damper, protected MaterialPropertyInterface
{
public:
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
  const Node *& _current_node;

  /// Quadrature point index
  unsigned int _qp;

  /// The current Newton increment
  VariableValue & _u_increment;
  /// Holds the current solution at the current node
  const VariableValue & _u;
};

#endif // NODALDAMPER_H
