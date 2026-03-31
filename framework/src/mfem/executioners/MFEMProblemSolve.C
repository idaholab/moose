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
  if (_mfem_problem.shouldSolve())
  {
    for (const auto & problem_operator : _problem_operators)
      problem_operator->Solve();

    // Short-circuit evaluation guarantees we only do one of p- or h-refinement between solves
    while (_mfem_problem.pRefine() || _mfem_problem.hRefine())
    {
      // Remove me: reconstruct the solver due to possible mfem/hypre bug
      if (_mfem_problem.getProblemData().jacobian_solver)
        _mfem_problem.getProblemData().jacobian_solver->constructSolver();

      // Reset gridfunctions
      for (const auto & problem_operator : _problem_operators)
        problem_operator->SetGridFunctions();

      // Solve again
      for (const auto & problem_operator : _problem_operators)
        problem_operator->Solve();
    }
  }

  _mfem_problem.displaceMesh();

  return true;
}

#endif
