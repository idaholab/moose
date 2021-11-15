//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorVectorMatPropElementalAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorVectorMatPropElementalAux);
registerMooseObject("MooseApp", FunctorADVectorMatPropElementalAux);

template <bool is_ad>
InputParameters
FunctorVectorMatPropElementalAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Evaluates a functor vector material property on the current element."
      "For finite volume, this evaluates the material property at the centroid.");
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The functor material property");
  params.addRequiredParam<unsigned int>("component", "Component of the vector material property");
  return params;
}

template <bool is_ad>
FunctorVectorMatPropElementalAuxTempl<is_ad>::FunctorVectorMatPropElementalAuxTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _mat_prop(getFunctor<GenericRealVectorValue<is_ad>>("mat_prop")),
    _component(getParam<unsigned int>("component"))
{
}

template <bool is_ad>
Real
FunctorVectorMatPropElementalAuxTempl<is_ad>::computeValue()
{
  return MetaPhysicL::raw_value(_mat_prop(_current_elem)(_component));
}
