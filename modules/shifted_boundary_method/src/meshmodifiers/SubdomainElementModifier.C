//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainElementModifier.h"
#include "SBMUtils.h"

#include <optional>

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainElementModifier);

InputParameters
SubdomainElementModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription("Assign subdomain ID based on geometric inclusion using per-subdomain "
                             "in-out checks provided by a subdomain_id_tester.");

  params.addRequiredParam<UserObjectName>(
      "subdomain_id_tester",
      "The UserObject (PointInSubdomainCheckUO) for subdomain in/out tests.");

  params.addRangeCheckedParam<Real>(
      "lambda",
      0.5,
      "lambda >= 0 & lambda <= 1",
      "Threshold ratio to decide between inside and outside if partially inside.");

  params.addRangeCheckedParam<int>(
      "qrule_order",
      9,
      "qrule_order >= 0 & qrule_order <= 10",
      "Quadrature order for generating the Gauss points to do the IN-OUT test to "
      "test whether the intercepted element belong to false intercepted element.");

  params.addParam<bool>(
      "mark_intercepted", false, "If true, mark the intercepted elements with the subdomain ID.");
  params.addParam<SubdomainID>("subdomain_id_intercepted",
                               "The subdomain ID to assign to intercepted elements.");

  return params;
}

SubdomainElementModifier::SubdomainElementModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _subdomain_id_tester(getUserObject<PointInSubdomainCheckUO>("subdomain_id_tester")),
    _lambda(getParam<Real>("lambda")),
    _qrule_order(static_cast<Order>(getParam<int>("qrule_order"))),
    _mark_intercepted(getParam<bool>("mark_intercepted")),
    _subdomain_id_intercepted(parameters.isParamValid("subdomain_id_intercepted")
                                  ? getParam<SubdomainID>("subdomain_id_intercepted")
                                  : Moose::INVALID_BLOCK_ID)
{
  if (_mark_intercepted && !isParamValid("subdomain_id_intercepted"))
    paramError("subdomain_id_intercepted",
               "This parameter is required when 'mark_intercepted' is true.");
}

SubdomainID
SubdomainElementModifier::computeSubdomainID()
{
  const Elem * elem = this->_current_elem;
  if (!elem)
    mooseError("SubdomainElementModifier: _current_elem is null!");

  const auto & all_checkers = _subdomain_id_tester.getAllSubdomainCheckers();
  if (all_checkers.empty())
    mooseError("SubdomainElementModifier: subdomain checker collection is empty!");

  std::optional<SubdomainID> fully_inside_subdomain;
  std::optional<SubdomainID> best_intercepted_subdomain;
  Real max_ratio = 0.0;

  for (const auto & [sub_id, checker_ptr] : all_checkers)
  {
    unsigned int num_inside_nodes = 0;
    for (const auto i : make_range(elem->n_nodes()))
      if (checker_ptr->sideness(elem->point(i)) != SurfaceSide::OUTSIDE)
        ++num_inside_nodes;

    if (num_inside_nodes == elem->n_nodes())
    {
      if (!fully_inside_subdomain || sub_id < *fully_inside_subdomain)
        fully_inside_subdomain = sub_id;
      continue;
    }

    // Copy the structured binding into a local variable; capturing a structured binding directly
    // in a lambda is only allowed from C++20 onward.
    auto * const checker = checker_ptr.get();
    const Real ratio_active = SBMUtils::activeElementFraction(
        *elem,
        _qrule_order,
        [&](const Point & point) { return checker->sideness(point) != SurfaceSide::OUTSIDE; });
    // All nodes may be outside even when a closed geometry lies within the element or its surface
    // crosses the element. Retain the checker if either nodes or quadrature points find activity.
    const bool has_active_region = num_inside_nodes != 0 || ratio_active > 0.0;
    if (!has_active_region)
      continue;

    if (!best_intercepted_subdomain || ratio_active > max_ratio ||
        (ratio_active == max_ratio && sub_id < *best_intercepted_subdomain))
    {
      max_ratio = ratio_active;
      best_intercepted_subdomain = sub_id;
    }
  }

  if (fully_inside_subdomain)
    return *fully_inside_subdomain;

  // If the element is outside all checked subdomains, leave its current subdomain unchanged by
  // returning Moose::INVALID_BLOCK_ID.
  if (!best_intercepted_subdomain)
    return Moose::INVALID_BLOCK_ID;

  if (_mark_intercepted)
    return _subdomain_id_intercepted;

  if (MooseUtils::absoluteFuzzyEqual(_lambda, 0))
    return Moose::INVALID_BLOCK_ID;
  if (MooseUtils::absoluteFuzzyEqual(_lambda, 1))
    return *best_intercepted_subdomain;

  return 1.0 - max_ratio > _lambda ? Moose::INVALID_BLOCK_ID : *best_intercepted_subdomain;
}
