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
class SubProblemInterface;
class SystemBase;
class MooseVariable;

template<>
InputParameters validParams<Damper>();

class Damper :
  public MooseObject,
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
  SubProblemInterface & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  MooseVariable & _var;

  const Elem * & _current_elem;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  /**
   * The current newton increment.
   */
  VariableValue & _u_increment;

  /**
   * Holds the current solution at the current quadrature point.
   */
  VariableValue & _u;

  /**
   * Holds the previous solution at the current quadrature point.
   */
  VariableValue & _u_old;

  /**
   * Holds the t-2 solution at the current quadrature point.
   */
  VariableValue & _u_older;

  /**
   * Holds the current solution gradient at the current quadrature point.
   */
  VariableGradient & _grad_u;

  /**
   * Holds the previous solution gradient at the current quadrature point.
   */
  VariableGradient & _grad_u_old;

  /**
   * Holds the t-2 solution gradient at the current quadrature point.
   */
  VariableGradient & _grad_u_older;

  /**
   * Holds the current solution second derivative at the current quadrature point.
   */
  VariableSecond & _second_u;
};
 
#endif
