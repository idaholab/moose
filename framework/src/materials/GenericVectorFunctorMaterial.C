//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericVectorFunctorMaterial.h"
#include "Function.h"

registerMooseObject("MooseApp", GenericVectorFunctorMaterial);
registerMooseObject("MooseApp", ADGenericVectorFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           GenericConstantVectorFunctorMaterial,
                           "06/30/2022 24:00",
                           GenericVectorFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           ADGenericConstantVectorFunctorMaterial,
                           "06/30/2022 24:00",
                           ADGenericVectorFunctorMaterial);

template <bool is_ad>
InputParameters
GenericVectorFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};
  params.addClassDescription(
      "FunctorMaterial object for declaring vector properties that are populated by "
      "evaluation of functor (constants, functions, variables, matprops) object.");
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  params.addParam<std::vector<MooseFunctorName>>("prop_values",
                                                 "The corresponding names of the "
                                                 "functors that are going to provide "
                                                 "the values for the vector material properties");
  return params;
}

template <bool is_ad>
GenericVectorFunctorMaterialTempl<is_ad>::GenericVectorFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<MooseFunctorName>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names * LIBMESH_DIM != num_values)
    mooseError("Number of prop_names times three must match the number of prop_values for a "
               "GenericVectorFunctorMaterial!");

  // Check that there is no name conflict, a common mistake with this object
  for (const auto i : make_range(num_names))
    for (const auto j : make_range(num_values))
      if (_prop_names[i] == _prop_values[j])
        paramError("prop_names",
                   "prop_names should not be the same as any of the prop_values. They"
                   " can both be functors, and functors may not have the same name.");

  _num_props = num_names;
  _functors.resize(num_values);

  for (const auto i : make_range(num_values))
    _functors[i] = &getFunctor<GenericReal<is_ad>>(_prop_values[i]);

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  for (const auto i : make_range(_num_props))
    addFunctorProperty<GenericRealVectorValue<is_ad>>(
        _prop_names[i],
        [this, i](const auto & r, const auto & t) -> GenericRealVectorValue<is_ad> {
          return {(*_functors[LIBMESH_DIM * i])(r, t),
                  (*_functors[LIBMESH_DIM * i + 1])(r, t),
                  (*_functors[LIBMESH_DIM * i + 2])(r, t)};
        },
        clearance_schedule);
}

template class GenericVectorFunctorMaterialTempl<false>;
template class GenericVectorFunctorMaterialTempl<true>;
