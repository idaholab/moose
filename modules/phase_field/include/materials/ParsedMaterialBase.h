/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PARSEDMATERIALBASE_H
#define PARSEDMATERIALBASE_H

#include "InputParameters.h"

// Forward Declarations
class ParsedMaterialBase;

template <>
InputParameters validParams<ParsedMaterialBase>();

/**
 * Helper class for ParsedMaterial and DerivativeParsedMaterial
 * to declare and read the input parameters.
 */
class ParsedMaterialBase
{
public:
  ParsedMaterialBase(const InputParameters & parameters);

protected:
  /// function expression
  std::string _function;

  /// constant vectors
  std::vector<std::string> _constant_names;
  std::vector<std::string> _constant_expressions;

  /// tolerance vectors
  std::vector<std::string> _tol_names;
  std::vector<Real> _tol_values;
};

#endif // PARSEDMATERIALBASE_H
