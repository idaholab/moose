//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "Executioner.h"
#include "MFEMProblemSolve.h"
#include "MFEMProblem.h"

InputParameters
MFEMProblemSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addClassDescription("Solve object for MFEM problems.");
  params.addParam<std::string>("device", "Run app on the chosen device.");
  MooseEnum assembly_levels("legacy full element partial none", "legacy", true);
  params.addParam<MooseEnum>("assembly_level", assembly_levels, "Matrix assembly level.");
  return params;
}

MFEMProblemSolve::MFEMProblemSolve(
    Executioner & ex,
    std::vector<std::shared_ptr<Moose::MFEM::ProblemOperatorBase>> & problem_operators)
  : SolveObject(ex),
    _mfem_problem(dynamic_cast<MFEMProblem &>(_problem)),
    _problem_operators(problem_operators)
{
  if (const auto compute_device = _app.getComputeDevice())
    _app.setMFEMDevice(*compute_device, Moose::PassKey<MFEMProblemSolve>());
  else
    _app.setMFEMDevice(isParamValid("device")    ? getParam<std::string>("device")
                       : _app.isUltimateMaster() ? "cpu"
                                                 : "",
                       Moose::PassKey<MFEMProblemSolve>());
}

bool
MFEMProblemSolve::solve()
{
  // FixedPointSolve::solve() is libMesh specific, so we need
  // to include all steps therein relevant to the MFEM backend here.

  bool converged = true;

  // need to back up multi-apps even when not doing fixed point iteration for recovering from failed
  // multiapp solve
  _mfem_problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.backupMultiApps(EXEC_TIMESTEP_BEGIN);
  _mfem_problem.backupMultiApps(EXEC_TIMESTEP_END);
  _mfem_problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_END);

  // Solve step begins
  _executioner.preSolve();
  _mfem_problem.execTransfers(EXEC_TIMESTEP_BEGIN);

  _mfem_problem.execute(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN, true);
  _mfem_problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);

  _mfem_problem.execMultiApps(EXEC_TIMESTEP_BEGIN, true);
  _mfem_problem.execute(EXEC_TIMESTEP_BEGIN);
  _mfem_problem.outputStep(EXEC_TIMESTEP_BEGIN);

  // Update warehouse active objects
  _mfem_problem.updateActiveObjects();

  if (_mfem_problem.shouldSolve())
    for (const auto & problem_operator : _problem_operators)
      problem_operator->Solve();
  _mfem_problem.displaceMesh();

  // Execute user objects, transfers, and multiapps at timestep end
  _mfem_problem.onTimestepEnd();
  _mfem_problem.execute(EXEC_TIMESTEP_END);
  _mfem_problem.execTransfers(EXEC_TIMESTEP_END);
  _mfem_problem.execMultiApps(EXEC_TIMESTEP_END, true);
  _executioner.postSolve();
  // Solve step ends

  if (converged)
  {
    // Fixed point iteration loop ends right above
    _mfem_problem.execute(EXEC_MULTIAPP_FIXED_POINT_END);
    _mfem_problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_END);
    _mfem_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_END, true);
    _mfem_problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_END);
  }

  return converged;
}
#endif
