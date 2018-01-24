//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPINTERFACE_H
#define SETUPINTERFACE_H

#include "MooseTypes.h"
#include "ExecFlagEnum.h"
#include "MooseEnum.h"
#include "InputParameters.h"

// Forward declarations
class InputParameters;
class MooseObject;
class SetupInterface;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<SetupInterface>();

class SetupInterface
{
public:
  SetupInterface(const MooseObject * moose_object);
  virtual ~SetupInterface();

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
   * Return the execute on MultiMooseEnum for this object.
   */
  const ExecFlagEnum & getExecuteOnEnum() const;

  /**
   * (DEPRECATED) Get the execution flag for the object
   * TODO: ExecFlagType
   */
  virtual const std::vector<ExecFlagType> & execFlags() const;

  /**
   * (DEPRECATED) Build and return the execution flags as a bitfield
   * TODO: ExecFlagType
   */
  ExecFlagType execBitFlags() const;

  /**
   * (DEPRECATED) Returns the available options for the 'execute_on' input parameters
   * TODO: ExecFlagType
   * @return A MooseEnum with the available 'execute_on' options, the default is 'residual'
   */
  static ExecFlagEnum getExecuteOptions();

private:
  /// Empty ExecFlagEnum for the case when the "execute_on" parameter is not included. This
  /// is private because others should not be messing with it.
  ExecFlagEnum _empty_execute_enum;

protected:
  /// Execute settings for this oejct.
  const ExecFlagEnum & _execute_enum;

  /// (DEPRECATED) execution flag (when is the object executed/evaluated) TODO: ExecFlagType
  const std::vector<ExecFlagType> _exec_flags;

  /// Reference to FEProblemBase
  const ExecFlagType & _current_execute_flag;

  // FEProblemBase::addMultiApp needs to reset the execution flags
  friend class FEProblemBase;
};

#endif /* SETUPINTERFACE_H */
