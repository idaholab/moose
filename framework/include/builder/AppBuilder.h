//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BuilderBase.h"

namespace Moose
{

/**
 * Helper for constructing an application.
 *
 * Enables:
 * - Parsing --type and Application/type ASAP to determine an app type
 * - Parsing and extracting Application/ parameters for the app
 * - Doing basic hit sanity checking as early as possible (before app construction)
 */
class AppBuilder : public BuilderBase
{
public:
  AppBuilder(std::shared_ptr<Parser> parser, const bool catch_parse_errors = false);

  /**
   * Build an application's parameters from command line arguments, with a
   * not-yet-decided type.
   *
   * This is used for constructing an application in main, where we do not
   * have a CommandLine object yet.
   *
   * The type is determined from either default_type or any of the command
   * line and input options (Application/type=, --type, or --app (deprecated)).
   * Once the type is determined, it starts with the valid parameters and
   * then specifies (in the parameters):
   *   _argc - The arg count from \p argc
   *   _argv - The arg count from \p argv
   *   _command_line - The built CommandLine from the arguments
   *
   * See buildParamsFromCommandLine() for information on what else is set.
   *
   * @param default_type The default application type to build if one can't be parsed
   * @param name The name of the application to build
   * @param argc Argument count
   * @param argv Argument vector
   * @param comm_world_in The MPI communicator
   * @return The built parameters
   */
  InputParameters buildParams(const std::string & default_type,
                              const std::string & name,
                              int argc,
                              char ** argv,
                              MPI_Comm comm_world_in);

  /**
   * Builds an application's parameters in place with a type already set
   * and CommandLine that is already filled in "_command_line".
   *
   * This is used for constructing applications not from main, such as a subapp,
   * where the CommandLine is duplicated from the parent application.
   *
   * The following parameters are specified:
   *   _app_name - The \p name
   *   _comm - The setup communicator from \p comm_world_in
   *   _app_builder_state - The state from the AppBuilder needed in the Builder
   *
   * This will merge all of the CLI args from the CommandLine and do an initial walk of the hit tree
   * to evaluate the brace expressions and look for duplicate variables. It will also populate
   * Application/... from both input and command line.
   *
   * @param name The name of the application to build
   * @param params The base parameters for the application
   * @param comm_world_in The MPI communicator
   */
  void buildParamsFromCommandLine(const std::string & name,
                                  InputParameters & params,
                                  MPI_Comm comm_world_in);

  /**
   * Helper struct for storing the state produced by build() that is needed
   * by the Builder later on
   */
  struct State
  {
    /// Root node for the parsed CLI hit arguments
    std::unique_ptr<hit::Node> cli_root;
    /// The variables that were extracted from Application/*
    std::set<std::string> extracted_vars;
  };

  /**
   * @return The recoverable (those that didn't affect the app parameter construction)
   * parse errors that were encountered
   *
   * Only collected when catch_parse_errors = true
   */
  const std::vector<hit::Error> & getParseErrors() const { return _parse_errors; }

private:
  /**
   * Performs the initial walk.
   *
   * Expands things and does some simple error checking
   */
  void initialWalk();

  /// Whether or not to catch recoverable parse errors
  const bool _catch_parse_errors;
  /// The recoverable parse errors
  std::vector<hit::Error> _parse_errors;
};

}
