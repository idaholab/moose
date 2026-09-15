//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class SolutionUserObjectBase;

/**
 * Tests whether SolutionUserObjectBase point-value caching correctly accounts
 * for source-subdomain restrictions.
 */
class TestSolutionPointValueCache : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestSolutionPointValueCache(const InputParameters & parameters);

  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  void initialSetup() override;

  Real getValue() const override;

protected:
  /// Source variable to evaluate
  const VariableName & _variable_name;

  /// Point located inside the source subdomain used to prime the cache
  const Point _prime_point;

  /// Point located outside the restricted source subdomain
  const Point _test_point;

  /// Source subdomain used for the restricted lookups
  const SubdomainID _source_subdomain;

  /// Solution user object containing the imported source solution
  const SolutionUserObjectBase * _solution_object_ptr = nullptr;
};
