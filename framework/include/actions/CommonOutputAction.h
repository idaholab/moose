//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "ReporterName.h"

#include <optional>

/**
 * Meta-action for creating common output object parameters
 * This action serves two purpose, first it adds common output object
 * parameters. Second, it creates the action (AddOutputAction) short-cuts
 * such as 'exodus=true' that result in the default output object of that
 * type to be created.
 * */
class CommonOutputAction : public Action
{
public:
  static InputParameters validParams();

  CommonOutputAction(const InputParameters & params);

  virtual void act() override;

  /**
   * Get the specific reporter names that we do not want to include
   * in general JSON output
   *
   * Whenever we create an additional JSON output for a specific reporter
   * value (the perf graph output for example), we do not want to output
   * those values in the standard JSON output
   */
  const std::vector<ReporterName> & getCommonReporterNames() const
  {
    return _common_reporter_names;
  }

  static const ReporterName perf_graph_json_reporter;

private:
  /**
   * Helper method for creating the short-cut actions
   * @param object_type String of the object type, i.e., the value of 'type=' in the input file
   * @param param_name The name of the input parameter that is responsible for creating, if any
   * @param from_params The parameters that are responsible for creating, if any
   * @param apply_params Additional parameters to apply, if any
   * @param object_name Override for the object name, if any (defaults to object type lowercased)
   */
  void create(const std::string & object_type,
              const std::optional<std::string> & param_name,
              const InputParameters * const from_params = nullptr,
              const InputParameters * const apply_params = nullptr,
              const std::optional<std::string> & object_name = {});

  /**
   * Check if a Console object that outputs to the screen has been defined
   * @return True if the a screen outputting Console objects
   */
  bool hasConsole();

  /// Parameters from the action being created (AddOutputAction)
  InputParameters _action_params;

  /// The reporter names that we do not want to include in general JSON output
  std::vector<ReporterName> _common_reporter_names;
};
