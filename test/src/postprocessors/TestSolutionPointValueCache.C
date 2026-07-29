//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSolutionPointValueCache.h"

#include "MooseUtils.h"
#include "SolutionUserObjectBase.h"

registerMooseObject("MooseTestApp", TestSolutionPointValueCache);

InputParameters
TestSolutionPointValueCache::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "The source variable to evaluate.");
  params.addRequiredParam<Point>(
      "prime_point", "A point inside the source subdomain used to prime the cache.");
  params.addRequiredParam<Point>(
      "test_point", "A point outside the restricted source subdomain.");
  params.addRequiredParam<SubdomainID>(
      "source_subdomain", "The source subdomain used for the restricted lookups.");
  params.addRequiredParam<UserObjectName>("solution",
                                          "The SolutionUserObject containing the source solution.");
  return params;
}

TestSolutionPointValueCache::TestSolutionPointValueCache(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _variable_name(getParam<VariableName>("variable")),
    _prime_point(getParam<Point>("prime_point")),
    _test_point(getParam<Point>("test_point")),
    _source_subdomain(getParam<SubdomainID>("source_subdomain"))
{
}

void
TestSolutionPointValueCache::initialSetup()
{
  _solution_object_ptr = &getUserObject<SolutionUserObjectBase>("solution");
}

Real
TestSolutionPointValueCache::getValue() const
{
  const std::set<subdomain_id_type> restricted_subdomains{_source_subdomain};

  // Cache a valid lookup restricted to the selected source subdomain.
  _solution_object_ptr->pointValue(
      _t, _prime_point, _variable_name, &restricted_subdomains);

  // Replace the cached point and values with an unrestricted lookup outside that subdomain.
  _solution_object_ptr->pointValue(_t, _test_point, _variable_name);

  // A fresh restricted evaluation at this point must fail because the point is outside the
  // selected source subdomain. The stale cache currently allows this call to return instead.
  return _solution_object_ptr->pointValue(
      _t, _test_point, _variable_name, &restricted_subdomains);
}
