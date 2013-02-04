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

#ifndef DAMPER_H
#define DAMPER_H

// Moose Includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"
#include "MaterialPropertyInterface.h"
// libMesh
#include "elem.h"
#include "quadrature.h"
#include "point.h"

//Forward Declarations
class Damper;
class Problem;
class SubProblem;
class SystemBase;
class MooseVariable;

template<>
InputParameters validParams<Damper>();

/**
 * Base class for deriving dampers
 *
 */
class Damper :
  public MooseObject,
  public SetupInterface,
  protected MaterialPropertyInterface
{
public:
  Damper(const std::string & name, InputParameters parameters);

  /**
   * Computes this Damper's damping for one element.
   */
  Real computeDamping();

protected:
  /**
   * This MUST be overridden by a child damper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping() = 0;

  SubProblem & _subproblem;
  SystemBase & _sys;

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
  VariableValue & _u;
  /// Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u;
};

#endif
