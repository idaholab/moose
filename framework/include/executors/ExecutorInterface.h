//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  Executor & getExecutor(const std::string & param_name) const;

  /**
   * Get an Executor based on its actual name
   *
   * @param executor_name the actual name of the Executor
   */
  Executor & getExecutorByName(const ExecutorName & executor_name) const;

private:
  const MooseObject * _ei_moose_object;

  MooseApp & _ei_app;
};
