//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "InputParameters.h"
#include "MooseVariableBase.h"
#include "Kernel.h"

/**
 * A helper class for defining the velocity as coupled variables
 * for the levelset equation.
 */
template <class T>
class LevelSetVelocityInterface : public T
{
public:
  static InputParameters validParams();

  LevelSetVelocityInterface(const InputParameters & parameters);

protected:
  /**
   * This method should be called when the velocity vector needs to be updated, this is not
   * done automatically to avoid populating a vector that is not used.
   */
  void computeQpVelocity();

  ///@{
  /// Coupled velocity variables
  const VariableValue & _velocity_x;
  const VariableValue & _velocity_y;
  const VariableValue & _velocity_z;
  ///@}

  ///@{
  /// Coupled velocity identifiers
  const unsigned int _x_vel_var;
  const unsigned int _y_vel_var;
  const unsigned int _z_vel_var;
  ///@}

  /// Storage for velocity vector
  RealVectorValue _velocity;
};

template <class T>
InputParameters
LevelSetVelocityInterface<T>::validParams()
{
  InputParameters parameters = emptyInputParameters();
  parameters.addCoupledVar(
      "velocity_x", 0, "The variable containing the x-component of the velocity front.");
  parameters.addCoupledVar(
      "velocity_y", 0, "The variable containing the y-component of the velocity front.");
  parameters.addCoupledVar(
      "velocity_z", 0, "The variable containing the z-component of the velocity front.");
  return parameters;
}

template <class T>
void
LevelSetVelocityInterface<T>::computeQpVelocity()
{
  _velocity(0) = _velocity_x[T::_qp];
  _velocity(1) = _velocity_y[T::_qp];
  _velocity(2) = _velocity_z[T::_qp];
}

template <class T>
LevelSetVelocityInterface<T>::LevelSetVelocityInterface(const InputParameters & parameters)
  : T(parameters),
    _velocity_x(T::coupledValue("velocity_x")),
    _velocity_y(T::coupledValue("velocity_y")),
    _velocity_z(T::coupledValue("velocity_z")),
    _x_vel_var(T::coupled("velocity_x")),
    _y_vel_var(T::coupled("velocity_y")),
    _z_vel_var(T::coupled("velocity_z"))
{
}
