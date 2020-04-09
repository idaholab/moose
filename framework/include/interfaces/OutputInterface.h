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
#include "MooseTypes.h"

// C++ includes
#include <set>
#include <string>

// Forward declerations
class InputParameters;
class MooseApp;
class OutputWarehouse;

template <typename T>
InputParameters validParams();

/**
 * A class to provide an common interface to objects requiring "outputs" option
 *
 * The 'outputs' option, when set restricts the output of the variable(s) associated with
 * this object to only occur on output objects listed.
 */
class OutputInterface
{
public:
  /**
   * Handles 'outputs' parameter for objects that desire control of variable outputs
   * @param parameters The parameters object holding data for the class to use.
   * @param build_list If false the buildOutputHideVariableList must be called explicitly, this
   * behavior
   *                   is required for automatic output of material properties
   */
  OutputInterface(const InputParameters & parameters, bool build_list = true);

  static InputParameters validParams();

  /**
   * Builds hide lists for output objects NOT listed in the 'outputs' parameter
   * @param variable_names A set of variables for which the 'outputs' parameter controls
   *
   * By default this is called by the constructor and passes the block name as the list of
   * variables. This needs to be called explicitly if the build_list flag is set to False
   * in the constructor. The latter cases is needed by the Material object to work correctly
   * with the automatic material output capability.
   */
  void buildOutputHideVariableList(std::set<std::string> variable_names);

  /**
   * Get the list of output objects that this class is restricted
   * @return A set of OutputNames
   */
  const std::set<OutputName> & getOutputs();

private:
  /// Reference the the MooseApp; neede for access to the OutputWarehouse
  MooseApp & _oi_moose_app;

  /// Reference to the OutputWarehouse for populating the Output object hide lists
  OutputWarehouse & _oi_output_warehouse;

  /// The set of Output object names listed in the 'outputs' parameter
  std::set<OutputName> _oi_outputs;
};
