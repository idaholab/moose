//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "EquationSystem.h"
#include "ComplexEquationSystem.h"
#include "MFEMContainers.h"
#include "CoefficientManager.h"
#include "MFEMLinearSolverBase.h"
#include "MFEMNonlinearSolverBase.h"
#include "MFEMRefinementMarker.h"

namespace Moose::MFEM
{
/// Base problem data struct.
struct ProblemData
{
public:
  ProblemData() = default;
  virtual ~ProblemData() { ode_solver.reset(); };

  std::shared_ptr<mfem::ParMesh> pmesh{nullptr};
  SubMeshes submeshes;
  CoefficientManager coefficients;

  std::unique_ptr<mfem::ODESolver> ode_solver{nullptr};
  mfem::BlockVector f;

  std::shared_ptr<EquationSystem> eqn_system{nullptr};
  std::shared_ptr<NonlinearSolverBase> nonlinear_solver{nullptr};

  std::shared_ptr<LinearSolverBase> jacobian_solver{nullptr};

  FECollections fecs;
  FESpaces fespaces;
  FESpaceHierarchies fespace_hierarchies;
  GridFunctions gridfunctions;
  TimeDerivativeMap time_derivative_map;
  ComplexGridFunctions cmplx_gridfunctions;

  std::shared_ptr<RefinementMarker> refiner;

  MPI_Comm comm;
  int myid;
  int num_procs;
};
} // namespace Moose::MFEM

#endif
