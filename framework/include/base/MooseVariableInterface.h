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

#ifndef MOOSEVARIABLEINTERFACE_H
#define MOOSEVARIABLEINTERFACE_H

#include "MooseVariable.h"
#include "InputParameters.h"

/**
 * Interface for objects that need to get values of MooseVariables
 */
class MooseVariableInterface
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   * @param var_param_name the parameter name where we will find the coupled variable name
   */
  MooseVariableInterface(const MooseObject * moose_object,
                         bool nodal,
                         std::string var_param_name = "variable");

  /**
   * Get the variable that this object is using.
   * @return The variable this object is using.
   */
  MooseVariable * mooseVariable();

  virtual ~MooseVariableInterface();

protected:
  /**
   * The value of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & value();

  /**
   * The old value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & valueOld();

  /**
   * The older value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & valueOlder();

  /**
   * The time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & dot();

  /**
   * The derivative of the time derivative of the variable this object is operating on
   * with respect to this variable's coefficients.
   *
   * This is useful for creating Jacobian entries for residual statements that use _u_dot
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & dotDu();

  /**
   * The gradient of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _grad_u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & gradient();

  /**
   * The old gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & gradientOld();

  /**
   * The older gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & gradientOlder();

  /**
   * The second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & second();

  /**
   * The old second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & secondOld();

  /**
   * The older second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & secondOlder();

  /**
   * The second derivative of the test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableTestSecond & secondTest();

  /**
   * The second derivative of the test function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the test function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableTestSecond & secondTestFace();

  /**
   * The second derivative of the trial function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariablePhiSecond & secondPhi();

  /**
   * The second derivative of the trial function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the trial function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariablePhiSecond & secondPhiFace();

  /// Whether or not this object is acting only at nodes
  bool _nodal;

  /// The variable this object is acting on
  MooseVariable * _variable;

protected:
  Assembly * _mvi_assembly;
};

#endif /* MOOSEVARIABLEINTERFACE_H */
