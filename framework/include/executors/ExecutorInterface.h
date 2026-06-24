//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Standard includes
#include <string>

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class PostprocessorName;
class MooseObject;
class Executor;
class MooseApp;

/**
 * Interface class for classes which interact with Postprocessors.
 * Provides the getPostprocessorValueXYZ() and related interfaces.
 */
class ExecutorInterface
{
public:
  ExecutorInterface(const MooseObject * moose_object);

  static InputParameters validParams();

  /**
   * Get an Executor based on a parameter name
   *
   * @param param_name The name of the parameter
   */
  template <typename T>
  T & getExecutor(const std::string & param_name) const;

  /**
   * Get an Executor based on its actual name
   *
   * @param executor_name the actual name of the Executor
   */
  template <typename T>
  T & getExecutorByName(const ExecutorName & executor_name) const;

private:
  const MooseObject * _ei_moose_object;

  MooseApp & _ei_app;
};

template <typename T>
T &
ExecutorInterface::getExecutor(const std::string & param_name) const
{
  return _ei_app.getExecutor<T>(_ei_moose_object->getParam<ExecutorName>(param_name));
}

template <typename T>
T &
ExecutorInterface::getExecutorByName(const ExecutorName & executor_name) const
{
  return _ei_app.getExecutor<T>(executor_name);
}
