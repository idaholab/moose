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

#ifndef MOOSEVARIABLEINTERFACEBASE_H
#define MOOSEVARIABLEINTERFACEBASE_H

#include "MooseVariable.h"
#include "InputParameters.h"
#include "SubProblem.h"
#include "Assembly.h"

/**
 * Interface for objects that need to get values of MooseVariables
 */
template<typename MooseVariableType, typename VariableValueType, typename VariableGradientType, typename VariableSecondType>
class MooseVariableInterfaceBase
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   * @param var_param_name the parameter name where we will find the coupled variable name
   */
  MooseVariableInterfaceBase(const MooseObject * moose_object, bool nodal, std::string var_param_name = "variable") :
      _nodal(nodal)
  {
    const InputParameters & parameters = moose_object->parameters();

    SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

    THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

    // Try the scalar version first
    std::string variable_name = parameters.getMooseType(var_param_name);
    if (variable_name == "")
      // When using vector variables, we are only going to use the first one in the list at the interface level...
      variable_name = parameters.getVecMooseType(var_param_name)[0];

    _variable = &problem.getVariable(tid, variable_name);

    _mvi_assembly = &problem.assembly(tid);
  }

  /**
   * Get the variable that this object is using.
   * @return The variable this object is using.
   */
  MooseVariable * mooseVariable()
  {
    return _variable;
  }

protected:
  /**
   * The value of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValueType & value()
  {
    if (_nodal)
      return _variable->nodalSln();
    else
      return _variable->sln();
  }

  /**
   * The old value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValueType & valueOld()
  {
    if (_nodal)
      return _variable->nodalSlnOld();
    else
      return _variable->slnOld();
  }

  /**
   * The older value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValueType & valueOlder()
  {
    if (_nodal)
      return _variable->nodalSlnOlder();
    else
      return _variable->slnOlder();
  }

  /**
   * The time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValueType & dot()
  {
    if (_nodal)
      return _variable->nodalSlnDot();
    else
      return _variable->uDot();
  }

  /**
   * The derivative of the time derivative of the variable this object is operating on
   * with respect to this variable's coefficients.
   *
   * This is useful for creating Jacobian entries for residual statements that use _u_dot
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValueType & dotDu()
  {
    if (_nodal)
      return _variable->nodalSlnDuDotDu();
    else
      return _variable->duDotDu();
  }

  /**
   * The gradient of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _grad_u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradientType & gradient()
  {
    if (_nodal)
      mooseError("Nodal variables do not have gradients");

    return _variable->gradSln();
  }

  /**
   * The old gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradientType & gradientOld()
  {
    if (_nodal)
      mooseError("Nodal variables do not have gradients");

    return _variable->gradSlnOld();
  }

  /**
   * The older gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradientType & gradientOlder()
  {
    if (_nodal)
      mooseError("Nodal variables do not have gradients");

    return _variable->gradSlnOlder();
  }

  /**
   * The second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecondType & second()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _variable->secondSln();
  }

  /**
   * The old second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecondType & secondOld()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _variable->secondSlnOld();
  }

  /**
   * The older second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecondType & secondOlder()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _variable->secondSlnOlder();
  }

  /**
   * The second derivative of the test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableTestSecond & secondTest()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _variable->secondPhi();
  }

  /**
   * The second derivative of the test function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the test function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableTestSecond & secondTestFace()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _variable->secondPhiFace();
  }

  /**
   * The second derivative of the trial function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariablePhiSecond & secondPhi()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _mvi_assembly->secondPhi();
  }

  /**
   * The second derivative of the trial function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the trial function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariablePhiSecond & secondPhiFace()
  {
    if (_nodal)
      mooseError("Nodal variables do not have second derivatives");

    return _mvi_assembly->secondPhiFace();
  }

  /// Whether or not this object is acting only at nodes
  bool _nodal;

  /// The variable this object is acting on
  MooseVariableType * _variable;

protected:
  Assembly * _mvi_assembly;
};


#endif /* MOOSEVARIABLEINTERFACEBASE_H */
