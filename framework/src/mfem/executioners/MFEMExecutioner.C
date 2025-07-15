//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMExecutioner.h"
#include "MFEMProblem.h"

InputParameters
MFEMExecutioner::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addClassDescription("Executioner for MFEM problems.");
  params.addParam<std::string>("device", "Run app on the chosen device.");
  MooseEnum assembly_levels("legacy full element partial none", "legacy", true);
  params.addParam<MooseEnum>(
      "assembly_level",
      assembly_levels,
      "Matrix assembly level. Options: legacy, full, element, partial, none.");
  return params;
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & params, MFEMProblem & mfem_problem)
  : _mfem_problem(mfem_problem), _problem_data(_mfem_problem.getProblemData())
{
  if (const auto compute_device = _app.getComputeDevice())
    _app.setMFEMDevice(*compute_device, Moose::PassKey<MFEMExecutioner>());
  else
    _app.setMFEMDevice(isParamValid("device")    ? getParam<std::string>("device")
                       : _app.isUltimateMaster() ? "cpu"
                                                 : "",
                       Moose::PassKey<MFEMExecutioner>());
}

void
MFEMExecutioner::solve()
{
  // FixedPointSolve::solve() is libMesh specific, so we need
  // to include all steps therein relevant to the MFEM backend here.

  // need to back up multi-apps even when not doing fixed point iteration for recovering from failed
  // multiapp solve
  _mfem_problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.backupMultiApps(EXEC_TIMESTEP_BEGIN);
  _mfem_problem.backupMultiApps(EXEC_TIMESTEP_END);
  _mfem_problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_END);

  // Fixed point iteration loop ends right above
  _mfem_problem.execute(EXEC_MULTIAPP_FIXED_POINT_END);
  _mfem_problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_END);
  _mfem_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_END, true);
  _mfem_problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_END);

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

  innerSolve();

  // Execute user objects, transfers, and multiapps at timestep end
  _mfem_problem.onTimestepEnd();
  _mfem_problem.execute(EXEC_TIMESTEP_END);
  _mfem_problem.execTransfers(EXEC_TIMESTEP_END);
  _mfem_problem.execMultiApps(EXEC_TIMESTEP_END, true);

  // Fixed point iteration loop ends right above
  _mfem_problem.execute(EXEC_MULTIAPP_FIXED_POINT_END);
  _mfem_problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_END);
  _mfem_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_END, true);
  _mfem_problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_END);
}
#endif
