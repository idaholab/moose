//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorElementalAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorElementalAux);
registerMooseObject("MooseApp", ADFunctorElementalAux);
registerMooseObjectRenamed("MooseApp",
                           FunctorMatPropElementalAux,
                           "06/30/2022 24:00",
                           FunctorElementalAux);
registerMooseObjectRenamed("MooseApp",
                           FunctorADMatPropElementalAux,
                           "06/30/2022 24:00",
                           ADFunctorElementalAux);

template <bool is_ad>
InputParameters
FunctorElementalAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Evaluates a functor (variable, function or functor material property) on the current "
      "element. For finite volume, this evaluates the material property at the centroid.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to evaluate");
  params.addParam<MooseFunctorName>("factor", 1, "A factor to apply on the functor");

  return params;
}

template <bool is_ad>
FunctorElementalAuxTempl<is_ad>::FunctorElementalAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _factor(getFunctor<GenericReal<is_ad>>("factor"))
{
}

template <bool is_ad>
Real
FunctorElementalAuxTempl<is_ad>::computeValue()
{
  const auto elem_arg = makeElemArg(_current_elem);
  return MetaPhysicL::raw_value(_factor(elem_arg)) * MetaPhysicL::raw_value(_functor(elem_arg));
}
