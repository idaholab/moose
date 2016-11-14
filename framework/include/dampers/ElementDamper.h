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

#ifndef ELEMENTDAMPER_H
#define ELEMENTDAMPER_H

// Moose Includes
#include "Damper.h"
#include "ParallelUniqueId.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariableBase.h"
#include "MooseVariableDependencyInterface.h"

//Forward Declarations
class ElementDamper;
class SubProblem;
class SystemBase;
class MooseVariable;
class Assembly;

template<>
InputParameters validParams<ElementDamper>();

/**
 * Base class for deriving element dampers
 */
class ElementDamper :
  public Damper,
  public MooseVariableDependencyInterface,
  public MaterialPropertyInterface
{
public:
  ElementDamper(const InputParameters & parameters);

  /**
   * Computes this Damper's damping for one element.
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

  /// Current element
  const Elem * & _current_elem;

  /// Quadrature point index
  unsigned int _qp;
  /// Quadrature points
  const MooseArray< Point > & _q_point;
  /// Quadrature rule
  QBase * & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;

  /// The current Newton increment
  VariableValue & _u_increment;
  /// Holds the current solution at the current quadrature point
  const VariableValue & _u;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u;
};

#endif //ELEMENTDAMPER_H
