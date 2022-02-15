//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorVectorElementalAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorVectorElementalAux);
registerMooseObject("MooseApp", ADFunctorVectorElementalAux);

template <bool is_ad>
InputParameters
FunctorVectorElementalAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Evaluates a vector functor (material property usually) on the current element."
      "For finite volume, this evaluates the vector functor at the centroid.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to evaluate");
  params.addRequiredParam<unsigned int>("component", "Component of the vector functor");
  params.addParam<MooseFunctorName>("factor", 1, "A factor to apply on the functor");

  return params;
}

template <bool is_ad>
FunctorVectorElementalAuxTempl<is_ad>::FunctorVectorElementalAuxTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<GenericRealVectorValue<is_ad>>("functor")),
    _component(getParam<unsigned int>("component")),
    _factor(getFunctor<GenericReal<is_ad>>("factor"))
{
}

template <bool is_ad>
Real
FunctorVectorElementalAuxTempl<is_ad>::computeValue()
{
  const auto elem_arg = makeElemArg(_current_elem);
  return MetaPhysicL::raw_value(_factor(elem_arg)) *
         MetaPhysicL::raw_value(_functor(elem_arg)(_component));
}
