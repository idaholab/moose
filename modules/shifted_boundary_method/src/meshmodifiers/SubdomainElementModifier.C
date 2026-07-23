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

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainElementModifier);

InputParameters
SubdomainElementModifier::validParams()
{
  InputParameters params = SBMElementSubdomainModifierBase::validParams();
  params.addClassDescription("Assign subdomain ID based on geometric inclusion using per-subdomain "
                             "in-out checks provided by a subdomain_id_tester.");

  params.addRequiredParam<UserObjectName>(
      "subdomain_id_tester",
      "The UserObject (PointInSubdomainCheckUO) for subdomain in/out tests.");

  return params;
}

SubdomainElementModifier::SubdomainElementModifier(const InputParameters & parameters)
  : SBMElementSubdomainModifierBase(parameters),
    _subdomain_id_tester(getUserObject<PointInSubdomainCheckUO>("subdomain_id_tester"))
{
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
  std::vector<std::pair<SubdomainID, Real>> intercepted_candidates;
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

    intercepted_candidates.emplace_back(sub_id, ratio_active);
    max_ratio = std::max(max_ratio, ratio_active);
  }

  if (fully_inside_subdomain)
    return *fully_inside_subdomain;

  // The checker collection is a std::unordered_map, so iteration order is not deterministic.
  // Compare every candidate against the fixed exact maximum, then resolve fuzzy ties by the
  // lowest subdomain ID.
  for (const auto & [sub_id, ratio_active] : intercepted_candidates)
    if (MooseUtils::absoluteFuzzyEqual(ratio_active, max_ratio) &&
        (!best_intercepted_subdomain || sub_id < *best_intercepted_subdomain))
      best_intercepted_subdomain = sub_id;

  // If the element is outside all checked subdomains, leave its current subdomain unchanged by
  // returning Moose::INVALID_BLOCK_ID.
  if (!best_intercepted_subdomain)
    return Moose::INVALID_BLOCK_ID;

  if (_mark_intercepted)
    return _subdomain_id_intercepted;

  return isInactive(max_ratio, _lambda) ? Moose::INVALID_BLOCK_ID : *best_intercepted_subdomain;
}
