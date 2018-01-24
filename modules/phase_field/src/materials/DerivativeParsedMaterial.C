//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
