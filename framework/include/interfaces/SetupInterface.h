//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "ExecFlagEnum.h"
#include "MooseEnum.h"
#include "InputParameters.h"

// Forward declarations
class InputParameters;
class MooseObject;
template <typename T>
InputParameters validParams();

class SetupInterface
{
public:
  SetupInterface(const MooseObject * moose_object);
  virtual ~SetupInterface();

  static InputParameters validParams();

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job
   */
  virtual void initialSetup();

  /**
   * Gets called at the beginning of the timestep before this object is asked to do its job
   */
  virtual void timestepSetup();

  /**
   * Gets called just before the Jacobian is computed and before this object is asked to do its job
   */
  virtual void jacobianSetup();

  /**
   * Gets called just before the residual is computed and before this object is asked to do its job
   */
  virtual void residualSetup();

  /**
   * Gets called when the subdomain changes (i.e. in a Jacobian or residual loop) and before this
   * object is asked to do its job
   */
  virtual void subdomainSetup();

  /**
   * Gets called in FEProblemBase::execute() for execute flags other than initial, timestep_begin,
   * nonlinear, linear and subdomain
   */
  virtual void customSetup(const ExecFlagType & /*exec_type*/) {}

  /**
   * Return the execute on MultiMooseEnum for this object.
   */
  const ExecFlagEnum & getExecuteOnEnum() const;

private:
  /// Empty ExecFlagEnum for the case when the "execute_on" parameter is not included. This
  /// is private because others should not be messing with it.
  ExecFlagEnum _empty_execute_enum;

protected:
  /// Execute settings for this object.
  const ExecFlagEnum & _execute_enum;

  /// Reference to FEProblemBase
  const ExecFlagType & _current_execute_flag;

  // FEProblemBase::addMultiApp needs to reset the execution flags
  friend class FEProblemBase;
};
