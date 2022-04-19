//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose
#include "MooseObject.h"
#include "MooseTypes.h"
#include "SetupInterface.h"
#include "Restartable.h"
#include "PerfGraphInterface.h"

class SubProblem;
class FEProblemBase;
class FEProblem;
class SystemBase;

namespace libMesh
{
class System;
class EquationSystems;
}

/**
 * Base class for all Transfer objects.
 *
 * Transfers are objects that take values from one Application
 * or System and put them in another Application or System.
 */
class Transfer : public MooseObject,
                 public SetupInterface,
                 public Restartable,
                 public PerfGraphInterface
{
public:
  Transfer(const InputParameters & parameters);
  virtual ~Transfer() = default;

  static InputParameters validParams();

  /**
   * Execute the transfer.
   */
  virtual void execute() = 0;

  /**
   * Method called at the beginning of the simulation for checking integrity or doing
   * one-time setup.
   */
  virtual void initialSetup() {}

  /**
   * Small helper function for finding the system containing the variable.
   *
   * Note that this implies that variable names are unique across all systems!
   *
   * @param es The EquationSystems object to be searched.
   * @param var_name The name of the variable you are looking for.
   */
  static System * find_sys(EquationSystems & es, const std::string & var_name);

  enum DIRECTION
  {
    TO_MULTIAPP,
    FROM_MULTIAPP,
    BETWEEN_MULTIAPP
  };

  /// Used to construct InputParameters
  static std::string possibleDirections() { return "to_multiapp from_multiapp between_multiapp"; }

  /// The directions this Transfer should be executed on
  const MultiMooseEnum & directions() { return _directions; }

  ///@{
  /// The current direction that this Transfer is going in.
  /// direction() is to be deprecated for currentDirection()
  MooseEnum direction() { return _direction; }
  MooseEnum currentDirection() { return _current_direction; }
  ///@}

  /// Set this Transfer to be executed in a given direction
  void setCurrentDirection(const int direction)
  {
    _current_direction = direction;
    _direction = direction;
  }

protected:
  SubProblem & _subproblem;
  FEProblemBase & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  ///@{
  /// The current direction that is being executed for this Transfer.
  /// _direction is to be deprecated for _current_direction
  MooseEnum _direction;
  MooseEnum _current_direction;
  ///@}

  /// The directions this Transfer is to be executed on
  MultiMooseEnum _directions;

public:
  const static Number OutOfMeshValue;
};
