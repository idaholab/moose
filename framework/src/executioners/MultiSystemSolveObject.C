//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addParamNamesToGroup("system_names", "Multiple solver system");
  return params;
}

MultiSystemSolveObject::MultiSystemSolveObject(Executioner & ex) : SolveObject(ex)
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
}
