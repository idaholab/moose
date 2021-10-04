//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericFunctionFunctorMaterial.h"
#include "Function.h"

registerMooseObject("MooseApp", GenericFunctionFunctorMaterial);
registerMooseObject("MooseApp", ADGenericFunctionFunctorMaterial);

template <bool is_ad>
InputParameters
GenericFunctionFunctorMaterialTempl<is_ad>::validParams()
{

  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.addClassDescription(
      "FunctorMaterial object for declaring properties that are populated by "
      "evaluation of Function object.");
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  params.addParam<std::vector<FunctionName>>("prop_values",
                                             "The corresponding names of the "
                                             "functions that are going to provide "
                                             "the values for the variables");
  return params;
}

template <bool is_ad>
GenericFunctionFunctorMaterialTempl<is_ad>::GenericFunctionFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<FunctionName>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError("Number of prop_names much match the number of prop_values for a "
               "GenericFunctionFunctorMaterial!");

  _num_props = num_names;
  _functions.resize(num_names);

  for (const auto i : make_range(_num_props))
    _functions[i] = &getFunctionByName<GenericReal<is_ad>>(_prop_values[i]);

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  for (const auto i : make_range(_num_props))
  {
    auto & prop = declareFunctorProperty<GenericReal<is_ad>>(_prop_names[i]);
    prop.setFunctor(
        _mesh, blockIDs(), [this, i](const auto & r, const auto & t) -> GenericReal<is_ad> {
          return (*_functions[i])(r, t);
        });
    prop.setCacheClearanceSchedule(clearance_schedule);
  }
}

template class GenericFunctionFunctorMaterialTempl<false>;
template class GenericFunctionFunctorMaterialTempl<true>;
