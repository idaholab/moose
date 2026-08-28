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

  const auto & all_checkers = _subdomain_id_tester.subdomainCheckers();
  if (all_checkers.empty())
    mooseError("SubdomainElementModifier: subdomain checker collection is empty!");

  // Compute the occupancy of the element with respect to each subdomain's in/out checker.
  std::vector<SBMUtils::SubdomainOccupancy> candidate_occupancies;
  candidate_occupancies.reserve(all_checkers.size());
  for (const auto & [subdomain_id, checker_ptr] : all_checkers)
  {
    const auto * const checker = checker_ptr.get();
    const auto is_in_domain = [checker](const Point & point)
    { return checker->sideness(point) != SurfaceGeometry::SurfaceSide::OUTSIDE; };
    candidate_occupancies.push_back(
        {subdomain_id, SBMUtils::elementDomainOccupancy(*elem, _qrule_order, is_in_domain)});
  }

  // Select the subdomain ID based on the measured occupancies and the intercepted-subdomain policy.
  const auto subdomain = SBMUtils::selectSubdomainFromOccupancies(
      candidate_occupancies, _intercepted_subdomain_policy, _lambda);

  // INVALID_BLOCK_ID tells ElementSubdomainModifier to retain the current subdomain.
  return subdomain.value_or(Moose::INVALID_BLOCK_ID);
}
