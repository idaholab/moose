#pragma once
#include "equation_system.h"
#include "MFEMContainers.h"
#include "outputs.h"
#include <fstream>
#include <iostream>
#include <memory>

namespace platypus
{

/// Base problem class.
class Problem
{
public:
  Problem() = default;
  virtual ~Problem() { _ode_solver.reset(); };

  std::shared_ptr<mfem::ParMesh> _pmesh{nullptr};
  platypus::BCMap _bc_map;
  platypus::Coefficients _coefficients;
  platypus::Outputs _outputs;

  std::unique_ptr<mfem::ODESolver> _ode_solver{nullptr};
  mfem::BlockVector _f;

  std::shared_ptr<mfem::Solver> _jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> _jacobian_solver{nullptr};
  std::shared_ptr<mfem::NewtonSolver> _nonlinear_solver{nullptr};

  platypus::FECollections _fecs;
  platypus::FESpaces _fespaces;
  platypus::GridFunctions _gridfunctions;

  mfem::Device _device;
  MPI_Comm _comm;
  int _myid;
  int _num_procs;

  /// Returns a pointer to the operator. See derived classes.
  [[nodiscard]] virtual mfem::Operator * GetOperator() const = 0;

  /// Virtual method to construct the operator. Call for default problems.
  virtual void ConstructOperator() = 0;
};

/// ProblemBuilder base class.
class ProblemBuilder
{
public:
  /// NB: delete empty constructor to allow only derived classes to be constructed.
  ProblemBuilder() = delete;

  // Virtual destructor required to prevent leaks.
  virtual ~ProblemBuilder() = default;

  virtual void RegisterGridFunctions() = 0;
  virtual void ConstructOperator() = 0;

  virtual void InitializeKernels(){};

  /// Returns a shared pointer to the problem.
  std::shared_ptr<platypus::Problem> ReturnProblem() { return _problem; }

protected:
  /// Protected constructor. Derived classes must call this constructor.
  ProblemBuilder(platypus::Problem * problem)
    : _problem(std::shared_ptr<platypus::Problem>(problem))
  {
  }

  /// Overridden in derived classes.
  [[nodiscard]] virtual platypus::Problem * GetProblem() const = 0;

  /// Helper template getter with safety check.
  template <class TDerivedProblem>
  [[nodiscard]] TDerivedProblem * GetProblem() const
  {
    if (!_problem)
    {
      MFEM_ABORT("platypus::Problem instance is NULL.");
    }

    return static_cast<TDerivedProblem *>(_problem.get());
  }

  /// Coefficient used in some derived classes.
  mfem::ConstantCoefficient _one_coef{1.0};

private:
  std::shared_ptr<platypus::Problem> _problem{nullptr};
};

/// Interface for problem builders that are constructing problems with an equation system.
class EquationSystemProblemBuilderInterface
{
public:
  EquationSystemProblemBuilderInterface() = default;
  virtual ~EquationSystemProblemBuilderInterface() = default;

  template <class T>
  void AddKernel(std::string var_name, std::shared_ptr<MFEMKernel<T>> kernel)
  {
    GetEquationSystem()->AddTrialVariableNameIfMissing(var_name);
    GetEquationSystem()->AddKernel(var_name, std::move(kernel));
  }

protected:
  /// Implemented in derived classes. Returns a pointer to the problem operator's equation system.
  [[nodiscard]] virtual platypus::EquationSystem * GetEquationSystem() const = 0;
};

} // namespace platypus
