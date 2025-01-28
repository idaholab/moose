//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"
#include "FunctionParserUtils.h"

class Function;

/**
 * Evaluates convergence from a parsed expression.
 */
class ParsedConvergence : public Convergence, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedConvergence(const InputParameters & parameters);

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

  virtual void initialSetup() override;

protected:
  usingFunctionParserUtilsMembers(false);

  /**
   * Initializes symbols used in the parsed expression
   */
  void initializeSymbols();
  void initializePostprocessorSymbol(unsigned int i);
  void initializeFunctionSymbol(unsigned int i);
  void initializeConvergenceSymbol(unsigned int i);
  void initializeConstantSymbol(unsigned int i);

  /**
   * Makes a parsed function
   *
   * @param[in] expression   expression to parse
   */
  SymFunctionPtr makeParsedFunction(const std::string & expression);

  /**
   * Updates non-constant symbol values
   *
   * @param[in] iter   Iteration index
   */
  void updateSymbolValues(unsigned int iter);
  void updatePostprocessorSymbolValues();
  void updateFunctionSymbolValues();
  void updateConvergenceSymbolValues(unsigned int iter);

  /**
   * Converts a Real value to a bool. Error results if value is not 0 or 1.
   *
   * @param[in] value  Real value to convert
   * @param[in] param  Name of the corresponding input parameter
   */
  bool convertRealToBool(Real value, const std::string & param) const;

  FEProblemBase & _fe_problem;

  /// User-defined symbols to use in parsed expression
  std::vector<std::string> _symbol_names;
  /// Corresponding symbol values (Convergence, Function, Postprocessor, or constant)
  std::vector<std::string> _symbol_values;

  /// Expression to parse for convergence
  const std::string _convergence_expression;
  /// Expression to parse for divergence
  const std::string _divergence_expression;

  /// Parsed function for convergence
  SymFunctionPtr _convergence_function;
  /// Parsed function for divergence
  SymFunctionPtr _divergence_function;

  /// Convergence function parameters
  std::vector<Real> _convergence_function_params;
  /// Divergence function parameters
  std::vector<Real> _divergence_function_params;

  /// Post-processor values in the provided symbols
  std::vector<const PostprocessorValue *> _pp_values;
  std::vector<unsigned int> _pp_indices;

  /// Functions in the provided symbols
  std::vector<Function *> _functions;
  std::vector<unsigned int> _function_indices;

  /// Convergences in the provided symbols
  std::vector<Convergence *> _convergences;
  std::vector<std::string> _convergence_symbol_names;
  std::vector<unsigned int> _convergence_indices;
};
