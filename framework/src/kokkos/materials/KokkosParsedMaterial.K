//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedMaterial.h"

registerKokkosMaterial("MooseApp", KokkosParsedMaterial);

InputParameters
KokkosParsedMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += ParsedObjectBase::validParams();

  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of the parsed material property");
  params.addParam<bool>(
      "use_dt", false, "Make  time step (dt) variable available in the function expression.");
  params.addClassDescription("Evaluates a property with a parsed expression.");

  return params;
}

KokkosParsedMaterial::KokkosParsedMaterial(const InputParameters & parameters)
  : Material(parameters),
    ParsedObjectBase(this),
    _prop(declareKokkosProperty<Real>("property_name"))
{
  if (getParam<bool>("use_dt"))
  {
    checkDuplicateSymbols(std::vector<std::string>{"dt"}, "use_dt");

    auto & dt = *_dt;
    addScalar("dt", &dt);
  }

  const auto & constant_names = getParam<std::vector<std::string>>("constant_names");
  const auto & postprocessor_names =
      getParam<std::vector<PostprocessorName>>("postprocessor_names");
  const auto & variable_names = coupledNames("coupled_variables");
  const auto & property_names =
      getParam<std::vector<MaterialPropertyName>>("material_property_names");
  const auto & function_names = getParam<std::vector<FunctionName>>("function_names");

  const auto & constant_expressions = parameters.get<std::vector<Real>>("constant_expressions");

  for (const auto i : make_range(constant_names.size()))
    addConstant(constant_names[i], constant_expressions[i]);

  for (const auto & pp : postprocessor_names)
    addScalar(pp, &getPostprocessorValueByName(pp));

  for (const auto i : make_range(variable_names.size()))
    addField(variable_names[i], kokkosCoupledValue("coupled_variables", i));

  for (const auto & prop : property_names)
    addProperty(prop, getKokkosMaterialPropertyByName<Real>(prop));

  for (const auto & func : function_names)
    addFunction(func, getKokkosFunctionByName(func));

  finalize();
}
