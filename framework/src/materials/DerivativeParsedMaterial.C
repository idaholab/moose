//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeParsedMaterial.h"

registerMooseObject("MooseApp", DerivativeParsedMaterial);
registerMooseObject("MooseApp", ADDerivativeParsedMaterial);

template <bool is_ad>
InputParameters
DerivativeParsedMaterialTempl<is_ad>::validParams()
{

  InputParameters params = DerivativeParsedMaterialHelperTempl<is_ad>::validParams();
  params += ParsedMaterialBase::validParams();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  return params;
}

template <bool is_ad>
DerivativeParsedMaterialTempl<is_ad>::DerivativeParsedMaterialTempl(
    const InputParameters & parameters)
  : DerivativeParsedMaterialHelperTempl<is_ad>(parameters,
                                               VariableNameMappingMode::USE_MOOSE_NAMES),
    ParsedMaterialBase(parameters, this)
{
  // Build function, take derivatives, optimize
  functionParse(_function,
                _constant_names,
                _constant_expressions,
                this->template getParam<std::vector<std::string>>("material_property_names"),
                this->template getParam<std::vector<PostprocessorName>>("postprocessor_names"),
                _tol_names,
                _tol_values,
                _functor_names,
                _functor_symbols);
}

// explicit instantiation
template class DerivativeParsedMaterialTempl<false>;
template class DerivativeParsedMaterialTempl<true>;
