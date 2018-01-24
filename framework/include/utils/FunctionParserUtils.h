//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONPARSERUTILS_H
#define FUNCTIONPARSERUTILS_H

#include "Moose.h"

#include "libmesh/fparser_ad.hh"

// C++ includes
#include <memory>

// Forward declartions
class FunctionParserUtils;
class InputParameters;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<FunctionParserUtils>();

class FunctionParserUtils
{
public:
  FunctionParserUtils(const InputParameters & parameters);

  /// Shorthand for an autodiff function parser object.
  typedef FunctionParserADBase<Real> ADFunction;

  /// Shorthand for an smart pointer to an autodiff function parser object.
  typedef std::shared_ptr<ADFunction> ADFunctionPtr;

  /// apply input paramters to internal feature flags of the parser object
  void setParserFeatureFlags(ADFunctionPtr &);

protected:
  /// Evaluate FParser object and check EvalError
  Real evaluate(ADFunctionPtr &);

  /// add constants (which can be complex expressions) to the parser object
  void addFParserConstants(ADFunctionPtr & parser,
                           const std::vector<std::string> & constant_names,
                           const std::vector<std::string> & constant_expressions);

  //@{ feature flags
  bool _enable_jit;
  bool _enable_ad_cache;
  bool _disable_fpoptimizer;
  bool _enable_auto_optimize;
  bool _fail_on_evalerror;
  //@}

  /// appropriate not a number value to return
  const Real _nan;

  /// table of FParser eval error codes
  static const char * _eval_error_msg[];

  /// Array to stage the parameters passed to the functions when calling Eval.
  std::vector<Real> _func_params;
};

#endif // FUNCTIONPARSERUTILS_H
