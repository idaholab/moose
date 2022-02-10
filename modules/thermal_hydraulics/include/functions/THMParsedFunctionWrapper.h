//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "MooseTypes.h"
#include "ControlData.h"
#include "libmesh/parsed_function.h"
#include <string>
#include <vector>

class Simulation;
class FEProblemBase;
class Function;

/**
 * A wrapper class for creating and evaluating parsed functions via the
 * libMesh::ParsedFunction interface for fparser. It has the same capability as
 * MooseParsedFunctionWrapper but adds ability to use control data values
 */
class THMParsedFunctionWrapper
{
public:
  /**
   * Class constructor
   * @param sim Reference to the Simulation object (provides access to ControlData)
   * @param feproblem Reference to the FEProblemBase object (provides access to Postprocessors)
   * @param function_str A string that contains the function to evaluate
   * @param vars A vector of variable names contained within the function
   * @param vals A vector of variable initial values, matching the variables defined in \c vars
   */
  THMParsedFunctionWrapper(Simulation & sim,
                           FEProblemBase & feproblem,
                           const std::string & function_str,
                           const std::vector<std::string> & vars,
                           const std::vector<std::string> & vals,
                           const THREAD_ID tid = 0);

  /**
   * Perform the evaluation of the libMesh::ParsedFunction
   */
  Real evaluate(Real t, const Point & p);

  /**
   * Get list of Real-valued control data objects
   */
  const std::vector<ControlData<Real> *> getRealControlData() { return _cd_real_vals; }

  /**
   * Get list of boolean-valued control data objects
   */
  const std::vector<ControlData<bool> *> getBoolControlData() { return _cd_bool_vals; }

private:
  /// Reference to the Simulation object
  Simulation & _sim;

  /// Reference to the FEProblemBase object
  FEProblemBase & _feproblem;

  /// Reference to the string containing the function to evaluate
  const std::string & _function_str;

  /// List of variables supplied from the user
  const std::vector<std::string> & _vars;

  /// List of the values for the variables supplied by the user
  const std::vector<std::string> & _vals_input;

  /// Storage for the initial values of _vars variables used by the libMesh::ParsedFunction object
  std::vector<Real> _initial_vals;

  /// Pointer to the libMesh::ParsedFunction object
  std::unique_ptr<ParsedFunction<Real>> _function_ptr;

  /// Stores _addr variable indices for each ControlData<Real> value
  std::vector<unsigned int> _cd_real_index;

  /// Vector of pointers to Real control data values this parsed function is using
  std::vector<ControlData<Real> *> _cd_real_vals;

  /// Stores _addr variable indices for each ControlData<bool> value
  std::vector<unsigned int> _cd_bool_index;

  /// Vector of pointers to bool control data values this parsed function is using
  std::vector<ControlData<bool> *> _cd_bool_vals;

  /// Stores _addr variable indices for each scalar variable value
  std::vector<unsigned int> _scalar_index;

  /// Vector of pointers to scalar variables values
  std::vector<const VariableValue *> _scalar_vals;

  /// Stores _addr variable indices for each Function
  std::vector<unsigned int> _function_index;

  /// Vector of Functions this parsed function is using
  std::vector<const Function *> _functions;

  /// Pointers to the variables that store the values of _vars inside the libMesh::ParsedFunction object
  std::vector<Real *> _addr;

  /// The thread id passed from owning object
  const THREAD_ID _tid;

  /**
   * Initialization method that prepares the _vars and _initial_vals for use
   * by the libMesh::ParsedFunction object allocated in the constructor
   */
  void initialize();

  /**
   * Updates scalar values for use in the libMesh::ParsedFunction
   */
  void update();

  /**
   * Updates function values for use in the libMesh::ParsedFunction
   */
  void updateFunctionValues(Real t, const Point & pt);

  /**
   * Updates control data values for use in the libMesh::ParsedFunction
   */
  void updateControlDataValues();
};
