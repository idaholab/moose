//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedMaterial.h"

registerMooseObject("MooseApp", ParsedMaterial);
registerMooseObject("MooseApp", ADParsedMaterial);

template <bool is_ad>
InputParameters
ParsedMaterialTempl<is_ad>::validParams()
{
  InputParameters params = ParsedMaterialHelper<is_ad>::validParams();
  params += ParsedMaterialBase::validParams();
  params.addClassDescription("Parsed expression Material.");
  return params;
}

template <bool is_ad>
ParsedMaterialTempl<is_ad>::ParsedMaterialTempl(const InputParameters & parameters)
  : ParsedMaterialHelper<is_ad>(parameters, VariableNameMappingMode::USE_MOOSE_NAMES),
    ParsedMaterialBase(parameters)
{
  // Build function and optimize
  functionParse(_function,
                _constant_names,
                _constant_expressions,
                this->template getParam<std::vector<std::string>>("material_property_names"),
                this->template getParam<std::vector<PostprocessorName>>("postprocessor_names"),
                _tol_names,
                _tol_values);
}

// explicit instantiation
template class ParsedMaterialTempl<false>;
template class ParsedMaterialTempl<true>;
