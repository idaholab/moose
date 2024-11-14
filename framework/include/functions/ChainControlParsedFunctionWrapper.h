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
#include "ChainControlData.h"
#include "libmesh/parsed_function.h"
#include <string>
#include <vector>

class MooseApp;
class FEProblemBase;
class Function;
class ChainControlDataSystem;

/**
 * Wraps libMesh::ParsedFunction for use in ChainControls
 */
class ChainControlParsedFunctionWrapper
{
public:
  /**
   * Constructor
   *
   * @param moose_app App
   * @param fe_problem Problem
   * @param function_str Function expression
   * @param symbol_names Symbols used in the function expression string
   * @param symbol_values Values pairing with \c symbol_names
   */
  ChainControlParsedFunctionWrapper(MooseApp & moose_app,
                                    FEProblemBase & fe_problem,
                                    const std::string & function_str,
                                    const std::vector<std::string> & symbol_names,
                                    const std::vector<std::string> & symbol_values,
                                    const THREAD_ID tid = 0);

  /**
   * Evaluates the libMesh::ParsedFunction
   *
   * @param t Time
   * @param p Spatial point
   */
  Real evaluate(Real t, const Point & p);

  /**
   * Get list of Real-valued control data objects
   */
  std::vector<ChainControlData<Real> *> getRealChainControlData()
  {
    return _real_control_data_values;
  }

  /**
   * Get list of boolean-valued control data objects
   */
  std::vector<ChainControlData<bool> *> getBoolChainControlData()
  {
    return _bool_control_data_values;
  }

private:
  /// App
  MooseApp & _moose_app;
  /// Problem
  FEProblemBase & _fe_problem;

  /// Function expression
  const std::string & _function_str;
  /// Symbols used in the function expression string
  const std::vector<std::string> & _symbol_names;
  /// Values pairing with \c symbol_names
  const std::vector<std::string> & _symbol_values;

  /// Initial value for each function input
  std::vector<Real> _initial_values;

  /// Wrapped libMesh::ParsedFunction
  std::unique_ptr<ParsedFunction<Real>> _function_ptr;

  /// _input_values index for each Real control data value
  std::vector<unsigned int> _real_control_data_indices;
  /// Real control data values
  std::vector<ChainControlData<Real> *> _real_control_data_values;

  /// _input_values index for each bool control data value
  std::vector<unsigned int> _bool_control_data_indices;
  /// bool control data values
  std::vector<ChainControlData<bool> *> _bool_control_data_values;

  /// _input_values index for each scalar variable value
  std::vector<unsigned int> _scalar_indices;
  /// Scalar variable values
  std::vector<const VariableValue *> _scalar_values;

  /// _input_values index for each function value
  std::vector<unsigned int> _function_indices;
  /// Function values
  std::vector<const Function *> _function_values;

  /// libMesh::ParsedFunction input values
  std::vector<Real *> _input_values;

  /// Thread id passed from owning object
  const THREAD_ID _tid;

  /// Chain control data system
  ChainControlDataSystem & _chain_control_data_system;

  /**
   * Gets initial value, address, and input index for each function input
   */
  void initializeFunctionInputs();

  /**
   * Updates scalar values in wrapped function
   */
  void updateScalarVariableValues();

  /**
   * Updates function values in wrapped function
   *
   * @param t Time
   * @param p Spatial point
   */
  void updateFunctionValues(Real t, const Point & pt);

  /**
   * Updates control data values in wrapped function
   */
  void updateChainControlDataValues();
};
