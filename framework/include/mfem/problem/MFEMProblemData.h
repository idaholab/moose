#ifdef MFEM_ENABLED

#pragma once
#include "EquationSystem.h"
#include "MFEMContainers.h"
#include "ObjectManager.h"
#include "PropertyManager.h"
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
  Moose::MFEM::ScalarCoefficientManager scalar_manager;
  Moose::MFEM::VectorCoefficientManager vector_manager;
  Moose::MFEM::MatrixCoefficientManager matrix_manager;
  Moose::MFEM::PropertyManager properties{scalar_manager, vector_manager, matrix_manager};

  std::unique_ptr<mfem::ODESolver> ode_solver{nullptr};
  mfem::BlockVector f;

  std::shared_ptr<Moose::MFEM::EquationSystem> eqn_system{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_solver{nullptr};
  std::shared_ptr<mfem::NewtonSolver> nonlinear_solver{nullptr};

  std::shared_ptr<MFEMSolverBase> mfem_preconditioner{nullptr};
  std::shared_ptr<MFEMSolverBase> mfem_solver{nullptr};

  Moose::MFEM::FECollections fecs;
  Moose::MFEM::FESpaces fespaces;
  Moose::MFEM::GridFunctions gridfunctions;

  MPI_Comm comm;
  int myid;
  int num_procs;
};

#endif
