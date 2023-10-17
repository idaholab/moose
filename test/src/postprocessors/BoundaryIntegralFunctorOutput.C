//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryIntegralFunctorOutput.h"
#include "MathFVUtils.h"

#include "metaphysicl/raw_type.h"

using namespace Moose;
using namespace FV;

registerMooseObject("MooseTestApp", BoundaryFunctorIntegralOutput);
registerMooseObject("MooseTestApp", ADBoundaryFunctorIntegralOutput);

template <bool is_ad>
InputParameters
BoundaryFunctorIntegralTempl<is_ad>::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable which this postprocessor integrates");
  params.addParam<BoundaryName>("boundary", "Boundary id to integrate with the functor");
  params.addParam<bool>(
      "compute_average", false, "Whether to compute an average instead of an integral");
  return params;
}

template <bool is_ad>
BoundaryFunctorIntegralTempl<is_ad>::BoundaryFunctorIntegralTempl(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  // We store the functor locally, not on the subproblem. This is not typical
  if (!getParam<bool>("compute_average"))
    _bif = std::make_unique<BoundaryIntegralFunctor<GenericReal<is_ad>>>(
        "boundary_integral",
        getFunctor<GenericReal<is_ad>>("variable"),
        clearance_schedule,
        getSubProblem().mesh(),
        getSubProblem().mesh().getBoundaryID(getParam<BoundaryName>("boundary")));
  else
    _baf = std::make_unique<BoundaryAverageFunctor<GenericReal<is_ad>>>(
        "boundary_integral",
        getFunctor<GenericReal<is_ad>>("variable"),
        clearance_schedule,
        getSubProblem().mesh(),
        getSubProblem().mesh().getBoundaryID(getParam<BoundaryName>("boundary")));
}

template <bool is_ad>
Real
BoundaryFunctorIntegralTempl<is_ad>::getValue() const
{
  // any argument should give the same result
  FaceArg random_face_arg = {getSubProblem().mesh().faceInfo(getSubProblem().mesh().elemPtr(0), 0),
                             Moose::FV::LimiterType::CentralDifference,
                             true,
                             true,
                             nullptr};

  if (!getParam<bool>("compute_average"))
    return MetaPhysicL::raw_value((*_bif)(random_face_arg, determineState()));
  else
    return MetaPhysicL::raw_value((*_baf)(random_face_arg, determineState()));
}

template class BoundaryFunctorIntegralTempl<false>;
template class BoundaryFunctorIntegralTempl<true>;
