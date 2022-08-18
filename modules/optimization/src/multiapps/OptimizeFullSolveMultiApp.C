//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizeFullSolveMultiApp.h"
#include "OptimizationAppTypes.h"

#include "FEProblemBase.h"

registerMooseObject("OptimizationApp", OptimizeFullSolveMultiApp);

InputParameters
OptimizeFullSolveMultiApp::validParams()
{
  InputParameters params = FullSolveMultiApp::validParams();
  params.addClassDescription("This is FullSolveMultiApp with some extra flags registered.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE,
                              OptimizationAppTypes::EXEC_FORWARD,
                              OptimizationAppTypes::EXEC_ADJOINT,
                              OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);
  params.addParam<ExecFlagEnum>(
      "execute_on", exec_enum, "List of flags indicating when this multiapp should solve.");

  params.addParam<bool>("reset_app", false, "Whether to reset app after each solve.");

  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;

  return params;
}

OptimizeFullSolveMultiApp::OptimizeFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters)
{
  // initilialize mpi for a single subapp per multiapp (this is the default that would usually
  // happen if use_positions = true but that doesn't make any sense for what I'm doing)
  init(1);
}

void
OptimizeFullSolveMultiApp::preTransfer(Real dt, Real target_time)
{
  if (getParam<bool>("reset_app"))
  {
    for (unsigned int i = 0; i < _my_num_apps; ++i)
      resetApp(i, target_time);
    _reset_happened = true;
    _fe_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
    initialSetup();
  }

  FullSolveMultiApp::preTransfer(dt, target_time);
}
