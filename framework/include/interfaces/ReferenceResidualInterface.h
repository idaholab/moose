//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "InputParameters.h"

class InputParameters;
class MooseObject;

/**
 * Interface class shared between ReferenceResidualProblem and ReferenceResidualConvergence.
 */
class ReferenceResidualInterface
{
public:
  static InputParameters validParams();

  ReferenceResidualInterface(const MooseObject * moose_object);

  /**
   * Add a set of variables that need to be grouped together. For use in
   * actions that create variables. This is templated for backwards compatibility to allow passing
   * in std::string or NonlinearVariableName.
   * @tparam T string type used for variable names
   * @param group_vars A set of solution variables that need to be grouped.
   */
  template <typename T>
  void addGroupVariables(const std::set<T> & group_vars);

protected:
  /// Name of variables that are grouped together to check convergence
  std::vector<std::vector<NonlinearVariableName>> _group_variables;

  /// True if any variables are grouped
  bool _use_group_variables;
};

template <typename T>
void
ReferenceResidualInterface::addGroupVariables(const std::set<T> & group_vars)
{
  _group_variables.push_back(
      std::vector<NonlinearVariableName>(group_vars.begin(), group_vars.end()));
  _use_group_variables = true;
}
