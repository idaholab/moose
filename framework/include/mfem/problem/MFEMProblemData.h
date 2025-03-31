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
  Moose::MFEM::TrackedScalarCoefficientFactory scalar_factory;
  Moose::MFEM::TrackedVectorCoefficientFactory vector_factory;
  Moose::MFEM::TrackedMatrixCoefficientFactory matrix_factory;
  platypus::CoefficientManager properties{scalar_factory, vector_factory, matrix_factory};

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
