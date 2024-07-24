//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

/**
 * Helper class for ParsedMaterial and DerivativeParsedMaterial
 * to declare and read the input parameters.
 */
class ParsedMaterialBase
{
public:
  static InputParameters validParams();

  ParsedMaterialBase(const InputParameters & parameters, const MooseObject * obj);

protected:
  /// Pointer to the MooseObject (to call paramError)
  const MooseObject * const _derived_object;

  /// function expression
  std::string _function;

  /// constant vectors
  std::vector<std::string> _constant_names;
  std::vector<std::string> _constant_expressions;

  /// tolerance vectors
  std::vector<std::string> _tol_names;
  std::vector<Real> _tol_values;

  /// Functor vectors (names, count, and symbols)
  std::vector<MooseFunctorName> _functor_names;
  std::vector<std::string> _functor_symbols;

  /**
   * Function to ensure that the names of constants, tolerances, and functors do not overlap with
   * each other and (optional) additional names.
   * @param reserved_names optional set of names additionaly not to be allowed.
   */
  void validateVectorNames(const std::set<std::string> & reserved_names = {});
};
