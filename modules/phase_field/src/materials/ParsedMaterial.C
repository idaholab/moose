/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ParsedMaterial.h"

template <>
InputParameters
validParams<ParsedMaterial>()
{
  InputParameters params = validParams<ParsedMaterialHelper>();
  params += validParams<ParsedMaterialBase>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

ParsedMaterial::ParsedMaterial(const InputParameters & parameters)
  : ParsedMaterialHelper(parameters, USE_MOOSE_NAMES), ParsedMaterialBase(parameters)
{
  // Build function and optimize
  functionParse(_function,
                _constant_names,
                _constant_expressions,
                getParam<std::vector<std::string>>("material_property_names"),
                _tol_names,
                _tol_values);
}
