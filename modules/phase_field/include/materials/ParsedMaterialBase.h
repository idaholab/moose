#ifndef PARSEDMATERIALBASE_H
#define PARSEDMATERIALBASE_H

#include "InputParameters.h"

// Forward Declarations
class ParsedMaterialBase;

template<>
InputParameters validParams<ParsedMaterialBase>();

/**
 * DerivativeFunctionMaterialBase child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class ParsedMaterialBase
{
public:
  ParsedMaterialBase(const std::string & name,
                     InputParameters parameters);

protected:
  /// function expression
  std::string _function;

  /// constant vectors
  std::vector<std::string> _constant_names;
  std::vector<std::string> _constant_expressions;

  /// tolerance vectors
  std::vector<std::string> _tol_names;
  std::vector<Real> _tol_values;

  /// material property names
  std::vector<std::string> _mat_prop_names;
};

#endif //PARSEDMATERIALBASE_H
