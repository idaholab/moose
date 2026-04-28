//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunctionParser.h"

#include "MooseObject.h"

namespace Moose::Kokkos
{

class ParsedObjectBase
{
public:
  static InputParameters validParams();

  ParsedObjectBase(const MooseObject * object);

protected:
  /**
   * Check if duplicate symbols were added
   * @param symbols The list of symbols
   * @param param The parameter name containing symbols
   */
  template <typename T>
  void checkDuplicateSymbols(const std::vector<T> & symbols, const std::string & param);

  /**
   * Add a constant
   * @param name The variable name
   * @param constant The constant value
   */
  void addConstant(const std::string & name, const Real constant);
  /**
   * Add a scalar variable
   * @param name The variable name
   * @param scalar The pointer to the scalar variable
   */
  void addScalar(const std::string & name, const Real & scalar);
  /**
   * Add a field variable
   * @param name The variable name
   * @param field The coupled field variable
   */
  void addField(const std::string & name, const VariableValue & field);
  /**
   * Add a material property
   * @param name The variable name
   * @param property The material property
   */
  void addProperty(const std::string & name, const MaterialProperty<Real> & property);
  /**
   * Add a function
   * @param name The variable name
   * @param function The function
   */
  void addFunction(const std::string & name, const Function & function);

  /**
   * Initialize symbols from parsed parameters. \p variable_names must be supplied by
   * the caller because Coupleable::coupledNames() is protected.
   */
  template <typename T>
  void initParsed(T * obj, const std::vector<VariableName> & variable_names);

  /**
   * Parsed expression
   */
  const std::string & _expression;
  /**
   * Parsed function builder
   */
  std::shared_ptr<RPNBuilder> _builder;
  /**
   * Parsed function evaluator
   */
  RPNEvaluator _evaluator;
  /**
   * Constants used in the parsed expression
   */
  std::unordered_map<std::string, Real> _constants;
  /**
   * Scalar variables used in the parsed expression
   */
  std::unordered_map<std::string, std::reference_wrapper<const Real>> _scalars;
  /**
   * Field variables used in the parsed expression
   */
  std::unordered_map<std::string, VariableValue> _fields;
  /**
   * Material properties used in the parsed expression
   */
  std::unordered_map<std::string, MaterialProperty<Real>> _properties;
  /**
   * Functions used in the parsed expression
   */
  std::unordered_map<std::string, Function> _functions;

private:
  /**
   * Finalize parsed function
   */
  void finalize();

  /**
   * Parsed object
   */
  const MooseObject * _parsed_object;
  /**
   * All symbols added to the parsed function
   */
  std::unordered_set<std::string> _all_symbols;
};

template <typename T>
void
ParsedObjectBase::checkDuplicateSymbols(const std::vector<T> & symbols, const std::string & param)
{
  for (const auto & symbol : symbols)
  {
    if (_all_symbols.count(symbol))
      _parsed_object->paramError(param, "Symbol '", symbol, "' was added multiple times.");

    _all_symbols.insert(symbol);
  }
}

template <typename T>
void
ParsedObjectBase::initParsed(T * obj, const std::vector<VariableName> & variable_names)
{
  const auto & constant_names = obj->template getParam<std::vector<std::string>>("constant_names");
  const auto & postprocessor_names =
      obj->template getParam<std::vector<PostprocessorName>>("postprocessor_names");
  const auto & property_names =
      obj->template getParam<std::vector<MaterialPropertyName>>("material_property_names");
  const auto & function_names = obj->template getParam<std::vector<FunctionName>>("function_names");
  const auto & constant_expressions =
      obj->template getParam<std::vector<Real>>("constant_expressions");

  for (const auto i : make_range(constant_names.size()))
    addConstant(constant_names[i], constant_expressions[i]);

  for (const auto & pp : postprocessor_names)
    addScalar(pp, obj->getPostprocessorValueByName(pp));

  for (const auto i : make_range(variable_names.size()))
    addField(variable_names[i], obj->kokkosCoupledValue("coupled_variables", i));

  for (const auto & prop : property_names)
    addProperty(prop, obj->template getKokkosMaterialPropertyByName<Real>(prop));

  for (const auto & func : function_names)
    addFunction(func, obj->getKokkosFunctionByName(func));

  finalize();
}

} // namespace Moose::Kokkos
