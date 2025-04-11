#pragma once
#include "equation_system.h"
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
  virtual ~MFEMProblemData() { _ode_solver.reset(); };

  std::shared_ptr<mfem::ParMesh> _pmesh{nullptr};
  platypus::ScalarCoefficientManager _scalar_manager;
  platypus::VectorCoefficientManager _vector_manager;
  platypus::MatrixCoefficientManager _matrix_manager;
  platypus::PropertyManager _properties{_scalar_manager, _vector_manager, _matrix_manager};
  platypus::BCMap _bc_map;

  std::unique_ptr<mfem::ODESolver> _ode_solver{nullptr};
  mfem::BlockVector _f;

  std::shared_ptr<platypus::EquationSystem> _eqn_system{nullptr};
  std::shared_ptr<mfem::Solver> _jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> _jacobian_solver{nullptr};
  std::shared_ptr<mfem::NewtonSolver> _nonlinear_solver{nullptr};

  platypus::FECollections _fecs;
  platypus::FESpaces _fespaces;
  platypus::GridFunctions _gridfunctions;

  MPI_Comm _comm;
  int _myid;
  int _num_procs;
};
