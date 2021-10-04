//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantVectorFunctorMaterial.h"

registerMooseObject("MooseApp", GenericConstantVectorFunctorMaterial);
registerMooseObject("MooseApp", ADGenericConstantVectorFunctorMaterial);

template <bool is_ad>
InputParameters
GenericConstantVectorFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Declares material properties based on names and vector values "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties. "
                                             "The vector lengths must be the same.");
  params.declareControllable("prop_values");
  return params;
}

template <bool is_ad>
GenericConstantVectorFunctorMaterialTempl<is_ad>::GenericConstantVectorFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_values != num_names * LIBMESH_DIM)
    mooseError("prop_values must be a equal to dim * number of prop_names for a "
               "GenericConstantVectorFunctorMaterial.");

  _num_props = num_names;
  _properties.resize(num_names);

  typedef GenericRealVectorValue<is_ad> ReturnType;

  for (unsigned int i = 0; i < _num_props; i++)
    _properties[i] = &declareFunctorProperty<ReturnType>(_prop_names[i]);

  for (unsigned int i = 0; i < _num_props; i++)
    _properties[i]->setFunctor(
        _mesh, this->blockIDs(), [this, i](const auto &, const auto &) -> ReturnType {
          ReturnType ret;
          for (unsigned int j = 0; j < LIBMESH_DIM; j++)
            ret(j) = _prop_values[i * LIBMESH_DIM + j];
          return ret;
        });
}

template class GenericConstantVectorFunctorMaterialTempl<false>;
template class GenericConstantVectorFunctorMaterialTempl<true>;
