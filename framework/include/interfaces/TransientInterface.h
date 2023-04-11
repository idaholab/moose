//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseFunctorArguments.h"

#define usingTransientInterfaceMembers                                                             \
  using TransientInterface::_t;                                                                    \
  using TransientInterface::_t_step;                                                               \
  using TransientInterface::_dt;                                                                   \
  using TransientInterface::_dt_old

// Forward declarations
class FEProblemBase;
class InputParameters;
class MooseObject;
template <typename T>
InputParameters validParams();

/**
 * Interface for objects that needs transient capabilities
 */
class TransientInterface
{
public:
  TransientInterface(const MooseObject * moose_object);
  static InputParameters validParams();
  virtual ~TransientInterface();

  bool isImplicit() { return _is_implicit; }

protected:
  /**
   * Create a functor state argument that corresponds to the implicit state of this object. If we
   * are implicit then we will return the current state. If we are not, then we will return the old
   * state
   */
  Moose::StateArg determineState() const;

  const InputParameters & _ti_params;

  FEProblemBase & _ti_feproblem;

  /**
   * If the object is using implicit or explicit form. This does NOT mean time scheme,
   * but which values are going to be used in the object - either from current time or old time.
   * Note that
   * even explicit schemes have implicit form (it is the time derivative "kernel")
   */
  bool _is_implicit;

  /// Time
  Real & _t;

  /// The number of the time step
  int & _t_step;

  /// Time step size
  Real & _dt;

  /// Size of the old time step
  Real & _dt_old;

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;

private:
  const std::string _ti_name;
};

inline Moose::StateArg
TransientInterface::determineState() const
{
  return _is_implicit ? Moose::currentState() : Moose::oldState();
}
