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

  ParsedMaterialBase(const InputParameters & parameters);

protected:
  /// function expression
  const std::string _function;

  /// constant vectors
  const std::vector<std::string> _constant_names;
  const std::vector<std::string> _constant_expressions;

  /// tolerance vectors
  const std::vector<std::string> _tol_names;
  const std::vector<Real> _tol_values;
};
