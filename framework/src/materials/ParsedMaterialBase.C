//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedMaterialBase.h"
#include "ParsedAux.h"
#include "MooseObject.h"

InputParameters
ParsedMaterialBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addCoupledVar("args", "Vector of variables used in the parsed function");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");

  // Constants and their values
  params.addParam<std::vector<std::string>>(
      "constant_names",
      std::vector<std::string>(),
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      std::vector<std::string>(),
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  // Variables with applied tolerances and their tolerance values
  params.addParam<std::vector<std::string>>("tol_names",
                                            std::vector<std::string>(),
                                            "Vector of variable names to be protected from "
                                            "being 0 or 1 within a tolerance (needed for log(c) "
                                            "and log(1-c) terms)");
  params.addParam<std::vector<Real>>("tol_values",
                                     std::vector<Real>(),
                                     "Vector of tolerance values for the variables in tol_names");

  // Functors and their symbols
  params.addParam<std::vector<MooseFunctorName>>(
      "functor_names", {}, "Functors to use in the parsed expression");
  params.addParam<std::vector<std::string>>(
      "functor_symbols",
      {},
      "Symbolic name to use for each functor in 'functor_names' in the parsed expression. If not "
      "provided, then the actual functor names will be used in the parsed expression.");

  // Material properties
  params.addParam<std::vector<std::string>>(
      "material_property_names",
      std::vector<std::string>(),
      "Vector of material properties used in the parsed function");

  // Postprocessors
  params.addParam<std::vector<PostprocessorName>>(
      "postprocessor_names",
      std::vector<PostprocessorName>(),
      "Vector of postprocessor names used in the parsed function");

  // Function expression
  params.addDeprecatedCustomTypeParam<std::string>(
      "function",
      "FunctionExpression",
      "Parsed function (see FParser) expression for the parsed material",
      "'function' is deprecated, use 'expression' instead");
  // TODO Make required once deprecation is handled, see #19119
  params.addCustomTypeParam<std::string>(
      "expression",
      "FunctionExpression",
      "Parsed function (see FParser) expression for the parsed material");

  return params;
}

ParsedMaterialBase::ParsedMaterialBase(const InputParameters & parameters, const MooseObject * obj)
  : _derived_object(obj)
{
  // get function expression
  _function = parameters.isParamValid("function") ? parameters.get<std::string>("function")
                                                  : parameters.get<std::string>("expression");

  // get constant vectors
  _constant_names = parameters.get<std::vector<std::string>>("constant_names");
  _constant_expressions = parameters.get<std::vector<std::string>>("constant_expressions");

  // get tolerance vectors
  _tol_names = parameters.get<std::vector<std::string>>("tol_names");
  _tol_values = parameters.get<std::vector<Real>>("tol_values");

  // get functor vectors
  _functor_names = parameters.get<std::vector<MooseFunctorName>>("functor_names");
  _functor_symbols = parameters.get<std::vector<std::string>>("functor_symbols");

  // validate all vector names (constants, tolerances, and functors)
  validateVectorNames();
}

void
ParsedMaterialBase::validateVectorNames(const std::set<std::string> & reserved_names)
{
  // helper method to raise an paramError
  auto raiseErr = [this](std::string param_name, std::string msg)
  {
    if (_derived_object != nullptr)
      _derived_object->paramError(param_name, msg);
    else
      mooseException(msg);
  };

  auto hasDuplicates = [](const std::vector<std::string> & values)
  {
    std::set<std::string> s(values.begin(), values.end());
    return values.size() != s.size();
  };

  // helper function to check if the name given is one of the constants
  auto isKnownConstantName = [this](const std::string & name)
  {
    return (
        _constant_names.size() &&
        (std::find(_constant_names.begin(), _constant_names.end(), name) != _constant_names.end()));
  };

  // helper function to check if the name given is one of the reserved_names
  auto isReservedName = [reserved_names](const std::string & name)
  { return reserved_names.find(name) != reserved_names.end(); };

  // check constants
  if (hasDuplicates(_constant_names))
    raiseErr("constant_names", "In the constants duplicate names are not permitted.");
  for (const auto & name : _constant_names)
  {
    if (isReservedName(name))
      raiseErr("constant_names", "In constants, the name '" + name + "' is not permitted.");
  }

  // check tolerance vectors
  if (hasDuplicates(_tol_names))
    raiseErr("tol_names", "In the tolerances duplicate names are not permitted.");

  // check functor vectors
  if (_functor_symbols.empty())
  {
    if (!_functor_names.empty())
      raiseErr("functor_names", "functor_symbols must be the same length as functor_names.");
  }
  else
  {
    if (_functor_symbols.size() != _functor_names.size())
      raiseErr("functor_names", "functor_symbols must be the same length as functor_names.");
    std::vector<std::string> names;
    if (_functor_symbols.empty())
      std::copy(_functor_names.begin(), _functor_names.end(), std::back_inserter(names));
    else
      std::copy(_functor_symbols.begin(), _functor_symbols.end(), std::back_inserter(names));
    if (hasDuplicates(names))
      raiseErr(_functor_symbols.empty() ? "functor_names" : "functor_symbols",
               "In functors, duplicate names are not permitted.");
    for (const auto & name : names)
    {
      if (isKnownConstantName(name))
        raiseErr(_functor_symbols.empty() ? "functor_names" : "functor_symbols",
                 "In functors, the name '" + name + "' is already in use as a constant.");
      if (isReservedName(name))
        raiseErr(_functor_symbols.empty() ? "functor_names" : "functor_symbols",
                 "In functors, the name '" + name + "' is not permitted.");
    }
  }
}
