//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "FunctionParserUtils.h"

/**
 * Computes a functor material from a parsed expression of other functors.
 */
template <bool is_ad>
class ParsedFunctorMaterialTempl : public FunctorMaterial, public FunctionParserUtils<is_ad>
{
public:
  static InputParameters validParams();

  ParsedFunctorMaterialTempl(const InputParameters & parameters);

protected:
  usingFunctionParserUtilsMembers(is_ad);

  /**
   * Builds the parsed function
   */
  void buildParsedFunction();

  /// Expression to parse for the new functor material
  const std::string & _expression;

  /// Functors to use in the parsed expression
  const std::vector<std::string> & _functor_names;

  /// Number of functors
  const unsigned int _n_functors;

  /// Symbolic name to use for each functor
  std::vector<std::string> _functor_symbols;

  /// Name to give the new functor material property
  const std::string & _property_name;

  /// The parsed function
  SymFunctionPtr _parsed_function;

  /// Functors
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _functors;
};

typedef ParsedFunctorMaterialTempl<false> ParsedFunctorMaterial;
typedef ParsedFunctorMaterialTempl<true> ADParsedFunctorMaterial;
