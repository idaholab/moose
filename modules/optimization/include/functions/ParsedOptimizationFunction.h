//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationFunction.h"
#include "ReporterInterface.h"

#include "libmesh/fparser_ad.hh"

class ParsedOptimizationFunction : public OptimizationFunction, public ReporterInterface
{
public:
  static InputParameters validParams();

  ParsedOptimizationFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;
  virtual std::vector<Real> parameterGradient(Real t, const Point & p) const override;

protected:
  /// Function to evaluate an inputted parser
  Real evaluateExpression(FunctionParserADBase<Real> & parser,
                          Real t,
                          const Point & p,
                          std::string name = "") const;

  /// Function expression passed to FParser
  const std::string & _expression;

  /// Parameters passed to FParser
  const std::vector<std::string> & _param_names;

  /// Vector containing parameter values
  const std::vector<Real> & _params;

  /// Variables passed to FParser
  const std::vector<std::string> & _symbol_names;

  /// Values passed by the user, they may be Reals for Postprocessors
  const std::vector<Real> & _symbol_values;

  /// Pointer to parsed function object
  std::unique_ptr<FunctionParserADBase<Real>> _parser;

  /// Pointers to parsed function objects representing derivative (first four are xyzt)
  std::vector<std::unique_ptr<FunctionParserADBase<Real>>> _derivative_parsers;
};
