//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableBase.h"

// Forward declarations
class Assembly;
class MooseObject;
template <typename T>
class MooseVariableField;
template <typename T>
class MooseVariableFE;
template <typename T>
class MooseVariableFV;
template <typename T>
class MooseLinearVariableFV;

/**
 * Interface for objects that need to get values of MooseVariables
 */
template <typename T>
class MooseVariableInterface
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   * @param var_param_name the parameter name where we will find the coupled variable name
   */
  MooseVariableInterface(
      const MooseObject * moose_object,
      bool nodal,
      std::string var_param_name = "variable",
      Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
      Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_ANY);

  /**
   * Get the variable that this object is using.
   * @return The variable this object is using.
   */
  MooseVariableBase * mooseVariableBase() const { return _var; };

  /**
   * Return the \p MooseVariableField object that this interface acts on
   */
  MooseVariableField<T> & mooseVariableField();

  /**
   * Return the \p MooseVariableFE object that this interface acts on
   */
  MooseVariableFE<T> * mooseVariable() const;

  /**
   * Return the \p MooseVariableFV object that this interface acts on
   */
  MooseVariableFV<T> * mooseVariableFV() const;

  /**
   * Return the \p MooseLinearVariableFV object that this interface acts on
   */
  MooseLinearVariableFV<T> * mooseLinearVariableFV() const;

  virtual ~MooseVariableInterface();

protected:
  /**
   * The value of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & value();

  /**
   * The old value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & valueOld();

  /**
   * The older value of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & valueOlder();

  /**
   * The time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & dot();

  /**
   * The second time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & dotDot();

  /**
   * The old time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & dotOld();

  /**
   * The old second time derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & dotDotOld();

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
   * The derivative of the second time derivative of the variable this object is operating on
   * with respect to this variable's coefficients.
   *
   * This is useful for creating Jacobian entries for residual statements that use _u_dotdot
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & dotDotDu();

  /**
   * The gradient of the variable this object is operating on.
   *
   * This is computed by default and should already be available as _grad_u
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & gradient();

  /**
   * The old gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & gradientOld();

  /**
   * The older gradient of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & gradientOlder();

  /**
   * The second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & second();

  /**
   * The old second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & secondOld();

  /**
   * The older second derivative of the variable this object is operating on.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & secondOlder();

  /**
   * The second derivative of the test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableTestSecond & secondTest();

  /**
   * The second derivative of the test function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the test function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableTestSecond & secondTestFace();

  /**
   * The second derivative of the trial function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariablePhiSecond & secondPhi();

  /**
   * The second derivative of the trial function on the current face.
   * This should be called in e.g. IntegratedBC when you need second
   * derivatives of the trial function function on the boundary.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariablePhiSecond & secondPhiFace();

  /// Whether or not this object is acting only at nodes
  bool _nodal;

  /// The variable this object is acting on
  MooseVariableBase * _var = nullptr;
  MooseVariableFE<T> * _variable = nullptr;
  MooseVariableFV<T> * _fv_variable = nullptr;
  MooseLinearVariableFV<T> * _linear_fv_variable = nullptr;
  MooseVariableField<T> * _field_variable = nullptr;

protected:
  Assembly * _mvi_assembly;

private:
  const MooseObject & _moose_object;
};

// Declare all the specializations, as the template specialization declaration below must know
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::value();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::valueOld();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::valueOlder();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::dot();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::dotDot();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::dotOld();
template <>
const VectorVariableValue & MooseVariableInterface<RealVectorValue>::dotDotOld();

// Prevent implicit instantiation in other translation units where these classes are used
extern template class MooseVariableInterface<Real>;
extern template class MooseVariableInterface<RealVectorValue>;
extern template class MooseVariableInterface<RealEigenVector>;
