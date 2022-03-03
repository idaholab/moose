//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericFunctionVectorMaterial.h"
#include "Function.h"

registerMooseObject("MooseApp", GenericFunctionVectorMaterial);
registerMooseObject("MooseApp", ADGenericFunctionVectorMaterial);

template <bool is_ad>
InputParameters
GenericFunctionVectorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Material object for declaring vector properties that are populated "
                             "by evaluation of Function objects.");
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  params.addParam<std::vector<FunctionName>>("prop_values",
                                             "The corresponding names of the "
                                             "functions that are going to provide "
                                             "the values for the variables, "
                                             "minor ordering by component");
  return params;
}

template <bool is_ad>
GenericFunctionVectorMaterialTempl<is_ad>::GenericFunctionVectorMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<FunctionName>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names * LIBMESH_DIM != num_values)
    mooseError("Number of prop_names (",
               num_names,
               ") times the libmesh dimension (",
               LIBMESH_DIM,
               ") must match the number of prop_values (",
               num_values,
               ") for a GenericFunctionVectorMaterial!");

  _num_props = num_names;

  _properties.resize(num_names);
  _functions.resize(num_names * LIBMESH_DIM);

  for (unsigned int i = 0; i < _num_props; i++)
  {
    _properties[i] = &declareGenericProperty<RealVectorValue, is_ad>(_prop_names[i]);
    for (const auto j : make_range(Moose::dim))
      _functions[i * LIBMESH_DIM + j] = &getFunctionByName(_prop_values[i * LIBMESH_DIM + j]);
  }
}

template <bool is_ad>
void
GenericFunctionVectorMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpFunctions();
}

template <bool is_ad>
void
GenericFunctionVectorMaterialTempl<is_ad>::computeQpProperties()
{
  computeQpFunctions();
}

template <bool is_ad>
void
GenericFunctionVectorMaterialTempl<is_ad>::computeQpFunctions()
{
  for (unsigned int i = 0; i < _num_props; i++)
    for (const auto j : make_range(Moose::dim))
      (*_properties[i])[_qp](j) = (*_functions[i * LIBMESH_DIM + j]).value(_t, _q_point[_qp]);
}

template class GenericFunctionVectorMaterialTempl<false>;
template class GenericFunctionVectorMaterialTempl<true>;
