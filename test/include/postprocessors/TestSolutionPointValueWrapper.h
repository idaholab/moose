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
 * Tests the policy-based point-value and point-gradient wrappers of
 * SolutionUserObjectBase.
 */
class TestSolutionPointValueWrapper : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestSolutionPointValueWrapper(const InputParameters & parameters);

  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  void initialSetup() override;

  Real getValue() const override;

protected:
  /// Name of the variable in the imported source solution
  const VariableName & _variable_name;

  /// Physical point at which to evaluate the source solution
  const Point _point;

  /// Whether to evaluate a gradient instead of a value
  const bool _evaluate_gradient;

  /// Gradient component returned when evaluating a gradient
  const unsigned int _gradient_component;

  /// Policy used to reduce multiple element-specific values
  const MooseEnum & _weighting_type;

  /// Optional numeric subdomain IDs in the imported source mesh
  const std::vector<subdomain_id_type> & _source_subdomain_ids;

  /// Imported solution user object
  const SolutionUserObjectBase * _solution_object_ptr = nullptr;
};
