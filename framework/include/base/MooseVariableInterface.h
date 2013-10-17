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
  MooseVariableInterface(InputParameters & parameters, bool nodal, std::string var_param_name = "variable");

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
  virtual VariableValue & value();

  /**
   * The old value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & valueOld();

  /**
   * The older value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & valueOlder();

  /**
   * The time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & dot();

  /**
   * The derivative of the time derivative of the variable this object is operating on
   * with respect to this variable's coefficients.
   *
   * This is useful for creating Jacobian entries for residual statements that use _u_dot
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & dotDu();

  /**
   * The gradient of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _grad_u
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & gradient();

  /**
   * The old gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & gradientOld();

  /**
   * The older gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & gradientOlder();

  /**
   * The second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & second();

  /**
   * The old second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & secondOld();

  /**
   * The older second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & secondOlder();

  /**
   * The second derivative of the test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableTestSecond & secondTest();

  /**
   * The second derivative of the shape function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariablePhiSecond & secondPhi();

  /// Whether or not this object is acting only at nodes
  bool _nodal;

  /// The variable this object is acting on
  MooseVariable * _variable;

protected:
  Assembly * _mvi_assembly;
};


/**
 * Enhances MooseVariableInterface interface provide values from neighbor elements
 *
 */
class NeighborMooseVariableInterface : public MooseVariableInterface
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   */
  NeighborMooseVariableInterface(InputParameters & parameters, bool nodal);

  virtual ~NeighborMooseVariableInterface();

protected:
  /**
   * The value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & neighborValue();

  /**
   * The old value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & neighborValueOld();

  /**
   * The older value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableValue & neighborValueOlder();

  /**
   * The gradient of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & neighborGradient();

  /**
   * The old gradient of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & neighborGradientOld();

  /**
   * The older gradient of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableGradient & neighborGradientOlder();

  /**
   * The second derivative of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & neighborSecond();

  /**
   * The old second derivative of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & neighborSecondOld();

  /**
   * The older second derivative of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableSecond & neighborSecondOlder();

  /**
   * The second derivative of the neighbor's test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariableTestSecond & neighborSecondTest();

  /**
   * The second derivative of the neighbor's shape function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual VariablePhiSecond & neighborSecondPhi();

};

#endif /* MOOSEVARIABLEINTERFACE_H */
