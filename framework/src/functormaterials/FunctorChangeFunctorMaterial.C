//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorChangeFunctorMaterial.h"

registerMooseObject("MooseApp", FunctorChangeFunctorMaterial);
registerMooseObject("MooseApp", ADFunctorChangeFunctorMaterial);

template <bool is_ad>
InputParameters
FunctorChangeFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addRequiredParam<MooseFunctorName>("functor", "Functor for which to compute change");
  MooseEnum change_over("time_step nonlinear fixed_point");
  change_over.addDocumentation("time_step", "Over the time step");
  change_over.addDocumentation("nonlinear", "Over the nonlinear iteration");
  change_over.addDocumentation("fixed_point", "Over the MultiApp fixed point iteration");
  params.addRequiredParam<MooseEnum>(
      "change_over", change_over, "Interval over which to compute the change");
  params.addRequiredParam<bool>("take_absolute_value",
                                "If true, take the absolute value of the change.");
  params.addRequiredParam<std::string>("prop_name",
                                       "The name to give the functor material property");

  params.addClassDescription(
      "Adds a functor material property that computes the change in a functor value over a time "
      "step, fixed point iteration, or nonlinear iteration.");

  return params;
}

template <bool is_ad>
FunctorChangeFunctorMaterialTempl<is_ad>::FunctorChangeFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _ref_state(referenceState(getParam<MooseEnum>("change_over"))),
    _take_absolute_value(getParam<bool>("take_absolute_value")),
    _prop_name(getParam<std::string>("prop_name"))
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  addFunctorProperty<GenericReal<is_ad>>(
      _prop_name,
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        mooseAssert(t == Moose::currentState(),
                    "The functor properties defined by (AD)FunctorChangeFunctorMaterial objects "
                    "may only be evaluated at the current state.");

        const auto change = _functor(r, t) - _functor(r, _ref_state);
        if (_take_absolute_value)
          return std::abs(change);
        else
          return change;
      },
      clearance_schedule);
}

template <bool is_ad>
Moose::StateArg
FunctorChangeFunctorMaterialTempl<is_ad>::referenceState(const MooseEnum & change_over) const
{
  if (change_over == "time_step")
    return Moose::oldState();
  else if (change_over == "nonlinear")
  {
    _fe_problem.needSolutionState(1, Moose::SolutionIterationType::Nonlinear);
    return Moose::previousNonlinearState();
  }
  else if (change_over == "fixed_point")
  {
    _fe_problem.needSolutionState(1, Moose::SolutionIterationType::FixedPoint);
    return Moose::previousFixedPointState();
  }
  else
    mooseError("Invalid value");
}

template class FunctorChangeFunctorMaterialTempl<false>;
template class FunctorChangeFunctorMaterialTempl<true>;
