//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TRANSFER_H
#define TRANSFER_H

// Moose
#include "MooseObject.h"
#include "MooseTypes.h"
#include "SetupInterface.h"
#include "Restartable.h"

// Forward declarations
class Transfer;
class SubProblem;
class FEProblemBase;
class FEProblem;
class SystemBase;

namespace libMesh
{
class System;
class EquationSystems;
}

template <>
InputParameters validParams<Transfer>();

/**
 * Base class for all Transfer objects.
 *
 * Transfers are objects that take values from one Application
 * or System and put them in another Application or System.
 */
class Transfer : public MooseObject, public SetupInterface, public Restartable
{
public:
  Transfer(const InputParameters & parameters);
  virtual ~Transfer() = default;

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

protected:
  SubProblem & _subproblem;
  FEProblemBase & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

public:
  const static Number OutOfMeshValue;
};

#endif /* TRANSFER_H */
