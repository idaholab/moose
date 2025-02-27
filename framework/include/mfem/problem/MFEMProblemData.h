#ifdef MFEM_ENABLED

#pragma once
#include "EquationSystem.h"
#include "MFEMContainers.h"
#include "ObjectManager.h"
#include "PropertyManager.h"
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
  MooseMFEM::ScalarCoefficientManager scalar_manager;
  MooseMFEM::VectorCoefficientManager vector_manager;
  MooseMFEM::MatrixCoefficientManager matrix_manager;
  MooseMFEM::PropertyManager properties{scalar_manager, vector_manager, matrix_manager};
  MooseMFEM::BCMap bc_map;

  std::unique_ptr<mfem::ODESolver> ode_solver{nullptr};
  mfem::BlockVector f;

  std::shared_ptr<MooseMFEM::EquationSystem> eqn_system{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> jacobian_solver{nullptr};
  std::shared_ptr<mfem::NewtonSolver> nonlinear_solver{nullptr};

  MooseMFEM::FECollections fecs;
  MooseMFEM::FESpaces fespaces;
  MooseMFEM::GridFunctions gridfunctions;

  MPI_Comm comm;
  int myid;
  int num_procs;
};

#endif
