//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryIntegralFunctorMaterial.h"
#include "MathFVUtils.h"
#include "BoundaryIntegrationFunctor.h"

#include "metaphysicl/raw_type.h"

using namespace Moose;
using namespace FV;

registerMooseObject("MooseApp", BoundaryIntegralFunctorMaterial);
registerMooseObject("MooseApp", ADBoundaryIntegralFunctorMaterial);

template <bool is_ad>
InputParameters
BoundaryIntegralFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("functor_name", "Name of the functor to be created");
  params.addRequiredParam<MooseFunctorName>(
      "functor_in", "The name of the functor which the boundary functor will integrate");
  params.addParam<BoundaryName>("integration_boundary", "Boundary to integrate the functor on");
  params.addParam<bool>(
      "compute_average", false, "Whether to compute an average instead of an integral");
  return params;
}

template <bool is_ad>
BoundaryIntegralFunctorMaterialTempl<is_ad>::BoundaryIntegralFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  // We store the functor locally, not on the subproblem. This is not typical
  if (!getParam<bool>("compute_average"))
    _bif = std::make_unique<BoundaryIntegralFunctor<GenericReal<is_ad>>>(
        "boundary_integral",
        getFunctor<GenericReal<is_ad>>("functor_in"),
        clearance_schedule,
        _fe_problem.mesh(),
        _fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("integration_boundary")));
  else
    _baf = std::make_unique<BoundaryAverageFunctor<GenericReal<is_ad>>>(
        "boundary_average",
        getFunctor<GenericReal<is_ad>>("functor_in"),
        clearance_schedule,
        _fe_problem.mesh(),
        _fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("integration_boundary")));

  // This is block-restricted, when it probably does not need to be
  if (!getParam<bool>("compute_average"))
    addFunctorProperty<GenericReal<is_ad>>(
        getParam<MooseFunctorName>("functor_name"),
        [this](const auto & r, const auto & t) -> GenericReal<is_ad> { return (*_bif)(r, t); },
        clearance_schedule);
  else
    addFunctorProperty<GenericReal<is_ad>>(
        getParam<MooseFunctorName>("functor_name"),
        [this](const auto & r, const auto & t) -> GenericReal<is_ad> { return (*_baf)(r, t); },
        clearance_schedule);
}

template class BoundaryIntegralFunctorMaterialTempl<false>;
template class BoundaryIntegralFunctorMaterialTempl<true>;
