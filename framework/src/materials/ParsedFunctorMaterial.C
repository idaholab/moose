//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedFunctorMaterial.h"

registerMooseObject("MooseApp", ParsedFunctorMaterial);
registerMooseObject("MooseApp", ADParsedFunctorMaterial);

template <bool is_ad>
InputParameters
ParsedFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = ParsedFunctorMaterialHelper<is_ad>::validParams();
  params += ParsedMaterialBase::validParams();

  params.addClassDescription("FunctorMaterial defined using a parsed expression.");
  return params;
}

template <bool is_ad>
ParsedFunctorMaterialTempl<is_ad>::ParsedFunctorMaterialTempl(const InputParameters & parameters)
  : ParsedFunctorMaterialHelper<is_ad>(parameters, VariableNameMappingMode::USE_MOOSE_NAMES),
    ParsedMaterialBase(parameters)
{
  // Build function and optimize
  this->functionParse(_function,
                      _constant_names,
                      _constant_expressions,
                      this->template getParam<std::vector<MooseFunctorName>>("functor_names"),
                      this->template getParam<std::vector<PostprocessorName>>("postprocessor_names"));

  // Set functor
  _property.setFunctor(
    _mesh, blockIDs(), [this](const auto & r, const auto & t) -> GenericReal<is_ad> {

      // insert other functor values
      auto nfunctors = _functors.size();
      for (MooseIndex(_functors) i = 0; i < nfunctors; ++i)
        _func_params[i + _nargs] = _functors[i](r, t);

      // insert postprocessor values
      // these are updated automatically as they are references to their value
      auto npps = _postprocessor_values.size();
      for (MooseIndex(_postprocessor_values) i = 0; i < npps; ++i)
        _func_params[i + _nargs + nfunctors] = *_postprocessor_values[i];

      return evaluate(_func_F);
    });
}

// explicit instantiation
template class ParsedFunctorMaterialTempl<false>;
template class ParsedFunctorMaterialTempl<true>;
