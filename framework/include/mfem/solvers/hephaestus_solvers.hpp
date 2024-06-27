#pragma once
#include "../common/pfem_extras.hpp"
#include "inputs.hpp"

namespace hephaestus
{

class DefaultH1PCGSolver : public mfem::HyprePCG
{
public:
  DefaultH1PCGSolver(const hephaestus::InputParameters & params, const mfem::HypreParMatrix & M)
    : mfem::HyprePCG(M),
      _amg(M),
      _tol(params.GetOptionalParam<float>("Tolerance", 1.0e-9)),
      _abstol(params.GetOptionalParam<float>("AbsTolerance", 1e-16)),
      _max_iter(params.GetOptionalParam<unsigned int>("MaxIter", 1000)),
      _print_level(params.GetOptionalParam<int>("PrintLevel", GetGlobalPrintLevel()))
  {

    _amg.SetPrintLevel(_print_level);
    SetTol(_tol);
    SetAbsTol(_abstol);
    SetMaxIter(_max_iter);
    SetPrintLevel(_print_level);
    SetPreconditioner(_amg);
  }
  mfem::HypreBoomerAMG _amg;
  double _tol;
  double _abstol;
  int _max_iter;
  int _print_level;
};

class DefaultJacobiPCGSolver : public mfem::HyprePCG
{
public:
  DefaultJacobiPCGSolver(const hephaestus::InputParameters & params, const mfem::HypreParMatrix & M)
    : mfem::HyprePCG(M),
      _jacobi(M),
      _tol(params.GetOptionalParam<float>("Tolerance", 1.0e-9)),
      _abstol(params.GetOptionalParam<float>("AbsTolerance", 1e-16)),
      _max_iter(params.GetOptionalParam<unsigned int>("MaxIter", 1000)),
      _print_level(params.GetOptionalParam<int>("PrintLevel", GetGlobalPrintLevel()))
  {

    SetTol(_tol);
    SetAbsTol(_abstol);
    SetMaxIter(_max_iter);
    SetPrintLevel(_print_level);
    SetPreconditioner(_jacobi);
  }
  mfem::HypreDiagScale _jacobi;
  double _tol;
  double _abstol;
  int _max_iter;
  int _print_level;
};

class DefaultHCurlPCGSolver : public mfem::HyprePCG
{
public:
  DefaultHCurlPCGSolver(const hephaestus::InputParameters & params,
                        const mfem::HypreParMatrix & M,
                        mfem::ParFiniteElementSpace * edge_fespace)
    : mfem::HyprePCG(M),
      _ams(M, edge_fespace),
      _tol(params.GetOptionalParam<float>("Tolerance", 1.0e-16)),
      _abstol(params.GetOptionalParam<float>("AbsTolerance", 1e-16)),
      _max_iter(params.GetOptionalParam<unsigned int>("MaxIter", 1000)),
      _print_level(params.GetOptionalParam<int>("PrintLevel", GetGlobalPrintLevel()))
  {

    _ams.SetSingularProblem();
    _ams.SetPrintLevel(_print_level);
    SetTol(_tol);
    SetAbsTol(_abstol);
    SetMaxIter(_max_iter);
    SetPrintLevel(_print_level);
    SetPreconditioner(_ams);
  }
  mfem::HypreAMS _ams;
  double _tol;
  double _abstol;
  int _max_iter;
  int _print_level;
};

class DefaultHCurlFGMRESSolver : public mfem::HypreFGMRES
{
public:
  DefaultHCurlFGMRESSolver(const hephaestus::InputParameters & params,
                           const mfem::HypreParMatrix & M,
                           mfem::ParFiniteElementSpace * edge_fespace)
    : mfem::HypreFGMRES(M),
      _ams(M, edge_fespace),
      _tol(params.GetOptionalParam<float>("Tolerance", 1e-16)),
      _max_iter(params.GetOptionalParam<unsigned int>("MaxIter", 100)),
      _k_dim(params.GetOptionalParam<unsigned int>("KDim", 10)),
      _print_level(params.GetOptionalParam<int>("PrintLevel", GetGlobalPrintLevel()))
  {

    _ams.SetSingularProblem();
    _ams.SetPrintLevel(_print_level);
    SetTol(_tol);
    SetMaxIter(_max_iter);
    SetKDim(_k_dim);
    SetPrintLevel(_print_level);
    SetPreconditioner(_ams);
  }
  mfem::HypreAMS _ams;
  double _tol;
  int _max_iter;
  int _k_dim;
  int _print_level;
};

class DefaultGMRESSolver : public mfem::HypreGMRES
{
public:
  DefaultGMRESSolver(const hephaestus::InputParameters & params, const mfem::HypreParMatrix & M)
    : mfem::HypreGMRES(M),
      _amg(M),
      _tol(params.GetOptionalParam<float>("Tolerance", 1e-16)),
      _abstol(params.GetOptionalParam<float>("AbsTolerance", 1e-16)),
      _max_iter(params.GetOptionalParam<unsigned int>("MaxIter", 1000)),
      _print_level(params.GetOptionalParam<int>("PrintLevel", GetGlobalPrintLevel()))
  {

    _amg.SetPrintLevel(_print_level);
    SetTol(_tol);
    SetAbsTol(_abstol);
    SetMaxIter(_max_iter);
    SetPrintLevel(_print_level);
    SetPreconditioner(_amg);
  }
  mfem::HypreBoomerAMG _amg;
  double _tol;
  double _abstol;
  int _max_iter;
  int _print_level;
};

class SuperLUSolver : public mfem::SuperLUSolver
{
public:
  SuperLUSolver(MPI_Comm comm, int npdep = 1) : mfem::SuperLUSolver(comm, npdep){};
  void SetOperator(const mfem::Operator & op) override
  {
    _a_superlu = std::make_unique<mfem::SuperLURowLocMatrix>(op);
    mfem::SuperLUSolver::SetOperator(*_a_superlu.get());
  }

private:
  std::unique_ptr<mfem::SuperLURowLocMatrix> _a_superlu{nullptr};
};

} // namespace hephaestus
