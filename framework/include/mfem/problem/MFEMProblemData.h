//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "EquationSystem.h"
#include "MFEMContainers.h"
#include "CoefficientManager.h"
#include "MFEMSolverBase.h"
#include <fstream>
#include <iostream>
#include <memory>

/// Base problem data struct.
struct MFEMProblemData
{
public:
  MFEMProblemData() = default;
  virtual ~MFEMProblemData() { ode_solver.reset(); };

  std::shared_ptr<mfem::ParMesh> pmesh{nullptr};
  Moose::MFEM::SubMeshes submeshes;
  Moose::MFEM::CoefficientManager coefficients;

  std::unique_ptr<mfem::ODESolver> ode_solver{nullptr};
  mfem::BlockVector f;

  std::shared_ptr<Moose::MFEM::EquationSystem> eqn_system{nullptr};
  std::shared_ptr<mfem::NewtonSolver> nonlinear_solver{nullptr};

  std::shared_ptr<MFEMSolverBase> jacobian_preconditioner{nullptr};
  std::shared_ptr<MFEMSolverBase> jacobian_solver{nullptr};

  Moose::MFEM::FECollections fecs;
  Moose::MFEM::FESpaces fespaces;
  Moose::MFEM::GridFunctions gridfunctions;

  MPI_Comm comm;
  int myid;
  int num_procs;
};

#endif
