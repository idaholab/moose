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
   * This MUST be overriden by a child damper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping() = 0;

  Problem & _problem;
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;                                       ///< Thread ID

  Moose::CoordinateSystemType & _coord_sys;             ///< Coordinate system

  MooseVariable & _var;                                 ///< Non-linear variable this damper works on

  const Elem * & _current_elem;                         ///< Current element

  unsigned int _qp;                                     ///< Quadrature point index
  const std::vector< Point > & _q_point;                ///< Quadrature points
  QBase * & _qrule;                                     ///< Quadrature rule
  const std::vector<Real> & _JxW;                       ///< Transformed Jacobina weights

  VariableValue & _u_increment;                         ///< The current newton increment
  VariableValue & _u;                                   ///< Holds the current solution at the current quadrature point
  VariableValue & _u_old;                               ///< Holds the previous solution at the current quadrature point
  VariableValue & _u_older;                             ///< Holds the t-2 solution at the current quadrature point

  VariableGradient & _grad_u;                           ///< Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_old;                       ///< Holds the previous solution gradient at the current quadrature point
  VariableGradient & _grad_u_older;                     ///< Holds the t-2 solution gradient at the current quadrature point

  VariableSecond & _second_u;                           ///< Holds the current solution second derivative at the current quadrature point
};

#endif
