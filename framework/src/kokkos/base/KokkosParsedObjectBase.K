//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedObjectBase.h"

#include "FEProblemBase.h"

namespace Moose::Kokkos
{

InputParameters
ParsedObjectBase::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression.");
  params.addParam<bool>(
      "use_dt", false, "Make time step (dt) variable available in the function expression.");

  // Constants and their values
  params.addParam<std::vector<std::string>>("constant_names",
                                            std::vector<std::string>(),
                                            "Vector of constants used in the parsed function.");
  params.addParam<std::vector<Real>>("constant_expressions",
                                     std::vector<Real>(),
                                     "Vector of values for the constants in constant_names.");

  // Postprocessors
  params.addParam<std::vector<PostprocessorName>>(
      "postprocessor_names",
      std::vector<PostprocessorName>(),
      "Vector of postprocessor names used in the parsed function.");

  // Field variables
  params.addCoupledVar("coupled_variables", "Vector of variables used in the parsed function.");

  // Material properties
  params.addParam<std::vector<MaterialPropertyName>>(
      "material_property_names",
      std::vector<MaterialPropertyName>(),
      "Vector of material properties used in the parsed function.");

  // Functions
  params.addParam<std::vector<FunctionName>>("function_names",
                                             std::vector<FunctionName>(),
                                             "Vector of functions used in the parsed function.");

  // Function expression
  params.addParam<std::string>("expression", "Parsed function expression.");

  return params;
}

ParsedObjectBase::ParsedObjectBase(const MooseObject * object)
  : _expression(object->parameters().get<std::string>("expression")),
    _builder(std::make_shared<Moose::Kokkos::RPNBuilder>(_expression, &object->_console)),
    _parsed_object(object)
{
  const auto & parameters = object->parameters();

  if (parameters.get<bool>("use_xyzt"))
  {
    _builder->addDefaultVariables();

    _all_symbols.insert("x");
    _all_symbols.insert("y");
    _all_symbols.insert("z");
    _all_symbols.insert("t");
  }

  if (parameters.get<bool>("use_dt"))
  {
    addScalar("dt", object->getMooseApp().feProblem().dt());

    _all_symbols.insert("dt");
  }

  const auto & constant_names = parameters.get<std::vector<std::string>>("constant_names");
  const auto & postprocessor_names =
      parameters.get<std::vector<PostprocessorName>>("postprocessor_names");
  const auto & variable_names = parameters.isParamValid("coupled_variables")
                                    ? parameters.get<std::vector<VariableName>>("coupled_variables")
                                    : std::vector<VariableName>();
  const auto & property_names =
      parameters.get<std::vector<MaterialPropertyName>>("material_property_names");
  const auto & function_names = parameters.get<std::vector<FunctionName>>("function_names");

  checkDuplicateSymbols(constant_names, "constant_names");
  checkDuplicateSymbols(postprocessor_names, "postprocessor_names");
  checkDuplicateSymbols(variable_names, "coupled_variables");
  checkDuplicateSymbols(property_names, "material_property_names");
  checkDuplicateSymbols(function_names, "function_names");
}

void
ParsedObjectBase::addConstant(const std::string & name, const Real constant)
{
  _builder->addVariable(name);

  _constants[name] = constant;
}

void
ParsedObjectBase::addScalar(const std::string & name, const Real & scalar)
{
  _builder->addVariable(name);

  _scalars.insert({name, scalar});
}

void
ParsedObjectBase::addField(const std::string & name, const VariableValue & field)
{
  _builder->addVariable(name);

  _fields[name] = field;
}

void
ParsedObjectBase::addProperty(const std::string & name, const MaterialProperty<Real> & property)
{
  _builder->addVariable(name);

  _properties[name] = property;
}

void
ParsedObjectBase::addFunction(const std::string & name, const Function & function)
{
  _builder->addVariable(name);

  _functions.insert({name, function});
}

void
ParsedObjectBase::finalize()
{
  for (const auto & [name, constant] : _constants)
    _builder->associateScalar(name, &constant);

  for (const auto & [name, scalar] : _scalars)
    _builder->associateScalar(name, &scalar.get());

  for (const auto & [name, field] : _fields)
    _builder->associateField(name, &field);

  for (const auto & [name, property] : _properties)
    _builder->associateProperty(name, &property);

  for (const auto & [name, function] : _functions)
    _builder->associateFunction(name, &function);

  _builder->build();
  _builder->finalize();

  // Should be called after finalization
  _evaluator.init(*_builder);
}

} // namespace Moose::Kokkos
