#ifdef MFEM_ENABLED

#pragma once
#include "EquationSystem.h"
#include "MFEMContainers.h"
#include "TrackedObjectFactory.h"
#include "CoefficientManager.h"
#include <fstream>
#include <iostream>
#include <memory>

/// Base problem data struct.
struct MFEMProblemData
{
public:
  MFEMProblemData() = default;
  virtual ~MFEMProblemData() { ode_solver.reset(); };

  std::shared_ptr<mfem::ParMesh> _pmesh{nullptr};
  Moose::MFEM::TrackedScalarCoefficientFactory scalar_manager;
  Moose::MFEM::TrackedVectorCoefficientFactory vector_manager;
  Moose::MFEM::TrackedMatrixCoefficientFactory matrix_manager;
  Moose::MFEM::CoefficientManager properties{scalar_manager, vector_manager, matrix_manager};

  std::unique_ptr<mfem::ODESolver> ode_solver{nullptr};
  mfem::BlockVector f;

  std::shared_ptr<Moose::MFEM::EquationSystem> eqn_system{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_solver{nullptr};
  std::shared_ptr<mfem::NewtonSolver> nonlinear_solver{nullptr};

  Moose::MFEM::FECollections fecs;
  Moose::MFEM::FESpaces fespaces;
  Moose::MFEM::GridFunctions gridfunctions;

  MPI_Comm comm;
  int myid;
  int num_procs;
};

#endif
