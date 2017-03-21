/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DerivativeParsedMaterial.h"

template <>
InputParameters
validParams<DerivativeParsedMaterial>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params += validParams<ParsedMaterialBase>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  return params;
}

DerivativeParsedMaterial::DerivativeParsedMaterial(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters, USE_MOOSE_NAMES), ParsedMaterialBase(parameters)
{
  // Build function, take derivatives, optimize
  functionParse(_function,
                _constant_names,
                _constant_expressions,
                getParam<std::vector<std::string>>("material_property_names"),
                _tol_names,
                _tol_values);
}
