//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiSystemSolveObject.h"
#include "FEProblemBase.h"
#include "LinearSystem.h"
#include "NonlinearSystem.h"

InputParameters
MultiSystemSolveObject::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<SolverSystemName>>(
      "system_names",
      "Names of the solver systems (both linear and nonlinear) that will be solved");
  // Multi-system fixed point
  // Defaults to false because of the difficulty of defining a good multi-system convergence
  // criterion, unless we add a default one to the simulation?
  params.addParam<bool>(
      "multi_system_fixed_point",
      false,
      "Whether to perform fixed point (Picard) iterations between the nonlinear systems.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "multi_system_fixed_point_relaxation_factor",
      {1.0},
      "multi_system_fixed_point_relaxation_factor>0 & multi_system_fixed_point_relaxation_factor<2",
      "Relaxation factor(s) applied to system solution updates during multi-system fixed point "
      "iterations; 1 disables relaxation. If one value is provided it is applied to every system; "
      "otherwise the vector must match the number/order of systems being solved.");
  params.addParam<ConvergenceName>(
      "multi_system_fixed_point_convergence",
      "Convergence object to determine the convergence of the multi-system fixed point iteration. "
      "If unspecified, defaults to checking that every system is converged (based on their own "
      "convergence criterion)");

  params.addParamNamesToGroup(
      "system_names multi_system_fixed_point multi_system_fixed_point_convergence "
      "multi_system_fixed_point_relaxation_factor",
      "Multiple solver system");
  return params;
}

MultiSystemSolveObject::MultiSystemSolveObject(Executioner & ex)
  : SolveObject(ex),
    _using_multi_sys_fp_iterations(getParam<bool>("multi_system_fixed_point")),
    _multi_sys_fp_convergence(nullptr) // has not been created yet

{
  // Retrieve pointers to all the systems from the problem by default
  const auto & nl_sys_names = _problem.getNonlinearSystemNames();
  const auto & linear_sys_names = _problem.getLinearSystemNames();
  if (!isParamValid("system_names"))
  {
    for (const auto & sys_name : nl_sys_names)
      _systems.push_back(&_problem.getSolverSystem(_problem.solverSysNum(sys_name)));
    for (const auto & sys_name : linear_sys_names)
      _systems.push_back(&_problem.getSolverSystem(_problem.solverSysNum(sys_name)));
    _num_nl_systems = nl_sys_names.size();
  }
  else
  {
    _num_nl_systems = 0;
    // Retrieve pointers to all the user-specified systems in the order that the user specified them
    for (const auto & sys_name : getParam<std::vector<SolverSystemName>>("system_names"))
    {
      if (std::find(nl_sys_names.begin(), nl_sys_names.end(), sys_name) != nl_sys_names.end())
      {
        _systems.push_back(&_problem.getSolverSystem(_problem.solverSysNum(sys_name)));
        _num_nl_systems++;
      }
      else if (std::find(linear_sys_names.begin(), linear_sys_names.end(), sys_name) !=
               linear_sys_names.end())
        _systems.push_back(&_problem.getSolverSystem(_problem.solverSysNum(sys_name)));
      else
        paramError("system_names",
                   "System '" + sys_name +
                       "' was not found in the Problem. Did you forget to declare it in the "
                       "[Problem] block?");
    }
  }

  if (_pars.isParamSetByUser("multi_system_fixed_point_relaxation_factor") &&
      !_using_multi_sys_fp_iterations)
    paramError("Can't use relaxation factors because multisystem fixed point iteration hasn't been "
               "enabled!");

  setupMultiSystemFixedPointRelaxationFactors();
}

void
MultiSystemSolveObject::setupMultiSystemFixedPointRelaxationFactors()
{
  _multi_sys_fp_relax_factors =
      getParam<std::vector<Real>>("multi_system_fixed_point_relaxation_factor");
  if (_multi_sys_fp_relax_factors.size() == 1)
    _multi_sys_fp_relax_factors.resize(_systems.size(), _multi_sys_fp_relax_factors[0]);
  else if (_multi_sys_fp_relax_factors.size() != _systems.size())
    paramError("multi_system_fixed_point_relaxation_factor",
               "Must provide either 1 value or " + Moose::stringify(_systems.size()) +
                   " values (one per system in the solve order).");
}
