//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IndicatorThresholdCriterion.h"

#include "DisplacedProblem.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"

#include "libmesh/elem.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"

#include <limits>

registerMooseObject("MooseApp", IndicatorThresholdCriterion);

InputParameters
IndicatorThresholdCriterion::validParams()
{
  InputParameters params = RemeshCriterion::validParams();

  params.addClassDescription("Remeshes when the field of an Adaptivity Indicator exceeds a "
                             "threshold, or when an element becomes small compared to its target "
                             "size.");

  params.addRequiredParam<IndicatorName>(
      "indicator",
      "The name of the [Adaptivity/Indicators] sub-block to measure, which is also the name of the "
      "CONSTANT MONOMIAL field that Indicator writes.");
  params.addRequiredRangeCheckedParam<Real>(
      "refine_threshold",
      "refine_threshold > 0",
      "The criterion fires when the largest value of 'indicator' over the active elements is above "
      "this.");
  params.addParam<VariableName>(
      "sizing_variable",
      "The target element size field, a CONSTANT MONOMIAL auxiliary variable holding the element "
      "size the remesher aims for. Only read by the over-refinement test, so it has to be supplied "
      "together with 'coarsen_fraction'.");
  params.addRangeCheckedParam<Real>(
      "coarsen_fraction",
      "coarsen_fraction > 0 & coarsen_fraction < 1",
      "The criterion fires when the diameter of an active element falls below this fraction of the "
      "value of 'sizing_variable' on that element. Requires 'sizing_variable'.");

  return params;
}

IndicatorThresholdCriterion::IndicatorThresholdCriterion(const InputParameters & parameters)
  : RemeshCriterion(parameters),
    _indicator(getParam<IndicatorName>("indicator")),
    _refine_threshold(getParam<Real>("refine_threshold")),
    _check_over_refinement(isParamValid("sizing_variable") && isParamValid("coarsen_fraction")),
    _sizing_variable(isParamValid("sizing_variable") ? getParam<VariableName>("sizing_variable")
                                                     : VariableName()),
    _coarsen_fraction(isParamValid("coarsen_fraction") ? getParam<Real>("coarsen_fraction") : 0)
{
  if (isParamValid("coarsen_fraction") && !isParamValid("sizing_variable"))
    paramError("coarsen_fraction",
               "The over-refinement test compares the diameter of an element to the target size "
               "field, so 'sizing_variable' has to be supplied together with 'coarsen_fraction'.");

  if (isParamValid("sizing_variable") && !isParamValid("coarsen_fraction"))
    paramError("sizing_variable",
               "The target size field is only read by the over-refinement test, so "
               "'coarsen_fraction' has to be supplied together with 'sizing_variable'.");
}

void
IndicatorThresholdCriterion::initialSetup()
{
  checkElementalField("indicator", _indicator);

  if (_check_over_refinement)
    checkElementalField("sizing_variable", _sizing_variable);
}

bool
IndicatorThresholdCriterion::shouldRemesh()
{
  const auto & indicator = evaluationVariable(_indicator);
  const MooseVariable * const sizing =
      _check_over_refinement ? &evaluationVariable(_sizing_variable) : nullptr;

  Real local_maximum = std::numeric_limits<Real>::lowest();
  bool local_over_refined = false;

  // A CONSTANT MONOMIAL variable has one degree of freedom per element, so getElementalValue()
  // returns the value of the whole element.
  // The loop is restricted to owned elements because a CONSTANT MONOMIAL dof belongs to the
  // element's own rank, and the reductions below restore the global answer
  for (const auto & elem : evaluationMesh().getMesh().active_local_element_ptr_range())
  {
    local_maximum = std::max(local_maximum, indicator.getElementalValue(elem));

    // Elem::hmax() is the largest vertex separation, which is the diameter of a straight sided
    // element
    if (sizing && elem->hmax() < _coarsen_fraction * sizing->getElementalValue(elem))
      local_over_refined = true;
  }

  // The over-refinement test compares every element to its own target size, so there is no single
  // measured quantity to reduce and the flag itself is reduced instead. Both reductions run on
  // every rank, and in the same order, because a rank local decision deadlocks the engine.
  comm().max(local_over_refined);

  return maximumAboveThreshold(local_maximum, _refine_threshold) || local_over_refined;
}

const MooseVariable &
IndicatorThresholdCriterion::evaluationVariable(const std::string & name) const
{
  if (&evaluationMesh() == &_mesh)
    return _fe_problem.getStandardVariable(/*tid=*/0, name);

  // evaluationMesh() returned the displaced mesh, so it has already checked that the displaced
  // problem exists
  return _fe_problem.getDisplacedProblem()->getStandardVariable(/*tid=*/0, name);
}

void
IndicatorThresholdCriterion::checkElementalField(const std::string & parameter,
                                                 const std::string & name) const
{
  const auto & fe_type = evaluationVariable(name).feType();
  if (fe_type.order != libMesh::CONSTANT || fe_type.family != libMesh::MONOMIAL)
    paramError(parameter,
               "The field '",
               name,
               "' is read as one value per element, which requires a MONOMIAL variable of CONSTANT "
               "order.");
}
