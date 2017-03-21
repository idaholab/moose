/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FUNCTIONPARSERUTILS_H
#define FUNCTIONPARSERUTILS_H

#include "InputParameters.h"

// Forward declartions
class FunctionParserUtils;

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
