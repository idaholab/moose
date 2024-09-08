//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedElemValueFunctorMaterial.h"
#include "FixedElemValueFunctor.h"
#include "MooseMeshUtils.h"

#include "metaphysicl/raw_type.h"

using namespace Moose;
using namespace FV;

registerMooseObject("MooseApp", FixedElemValueFunctorMaterial);
registerMooseObject("MooseApp", ADFixedElemValueFunctorMaterial);

template <bool is_ad>
InputParameters
FixedElemValueFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("functor_name", "Name of the functor to be created");
  params.addRequiredParam<MooseFunctorName>(
      "functor_in", "The name of the functor which the boundary functor will integrate");
  params.addRequiredParam<unsigned int>("elem_id", "Fixed element id to evaluate the functor on");

  return params;
}

template <bool is_ad>
FixedElemValueFunctorMaterialTempl<is_ad>::FixedElemValueFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  // We store the functor locally, not on the subproblem. This is not typical
  _ef = std::make_unique<FixedElemValueFunctor<GenericReal<is_ad>>>(
      getParam<MooseFunctorName>("functor_name"),
      getFunctor<GenericReal<is_ad>>("functor_in"),
      clearance_schedule,
      _mesh,
      getParam<unsigned int>("elem_id"));

  // This is block-restricted, when it probably does not need to be
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("functor_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad> { return (*_ef)(r, t); },
      clearance_schedule);
}

template class FixedElemValueFunctorMaterialTempl<false>;
template class FixedElemValueFunctorMaterialTempl<true>;
