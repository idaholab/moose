//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "ADFParser.h"

#include "libmesh/fparser_ad.hh"

// C++ includes
#include <memory>

#define usingFunctionParserUtilsMembers(T)                                                         \
  using typename FunctionParserUtils<T>::SymFunction;                                              \
  using typename FunctionParserUtils<T>::SymFunctionPtr;                                           \
  using typename FunctionParserUtils<T>::FailureMethod;                                            \
  using FunctionParserUtils<T>::evaluate;                                                          \
  using FunctionParserUtils<T>::setParserFeatureFlags;                                             \
  using FunctionParserUtils<T>::addFParserConstants;                                               \
  using FunctionParserUtils<T>::_enable_jit;                                                       \
  using FunctionParserUtils<T>::_enable_ad_cache;                                                  \
  using FunctionParserUtils<T>::_disable_fpoptimizer;                                              \
  using FunctionParserUtils<T>::_enable_auto_optimize;                                             \
  using FunctionParserUtils<T>::_eval_error_msg;                                                   \
  using FunctionParserUtils<T>::_func_params

// Helper class to pic the correct function parser
template <bool is_ad>
struct GenericSymFunctionTempl
{
  typedef FunctionParserADBase<Real> type;
};
template <>
struct GenericSymFunctionTempl<true>
{
  typedef ADFParser type;
};
template <bool is_ad>
using GenericSymFunction = typename GenericSymFunctionTempl<is_ad>::type;

// Forward declartions
class InputParameters;

template <bool is_ad = false>
class FunctionParserUtils
{
public:
  static InputParameters validParams();

  FunctionParserUtils(const InputParameters & parameters);

  /// Shorthand for an autodiff function parser object.
  typedef GenericSymFunction<is_ad> SymFunction;

  /// Shorthand for an smart pointer to an autodiff function parser object.
  typedef std::shared_ptr<SymFunction> SymFunctionPtr;

  /// apply input paramters to internal feature flags of the parser object
  void setParserFeatureFlags(SymFunctionPtr &);

protected:
  /// Evaluate FParser object and check EvalError
  GenericReal<is_ad> evaluate(SymFunctionPtr &, const std::string & object_name = "");

  /// add constants (which can be complex expressions) to the parser object
  void addFParserConstants(SymFunctionPtr & parser,
                           const std::vector<std::string> & constant_names,
                           const std::vector<std::string> & constant_expressions);

  //@{ feature flags
  bool _enable_jit;
  bool _enable_ad_cache;
  bool _disable_fpoptimizer;
  bool _enable_auto_optimize;
  //@}

  /// Enum for failure method
  const enum class FailureMethod { nan, nan_warning, error, exception } _evalerror_behavior;

  /// appropriate not a number value to return
  const Real _quiet_nan;

  /// table of FParser eval error codes
  static const char * _eval_error_msg[];

  /// Array to stage the parameters passed to the functions when calling Eval.
  std::vector<GenericReal<is_ad>> _func_params;
};
