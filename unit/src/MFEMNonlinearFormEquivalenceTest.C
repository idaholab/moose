//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMMassKernel.h"
#include "MFEMNLDiffusionKernel.h"
#include "MFEMDomainLFKernel.h"
#include "MFEMScalarDirichletBC.h"
#include "EquationSystem.h"
#include "libmesh/int_range.h"

#include <limits>

namespace
{
/**
 * Records the residual norm reported at each nonlinear iteration, so that two solves can be
 * compared iteration by iteration rather than only on their final result.
 */
class ResidualHistory : public mfem::IterativeSolverMonitor
{
public:
  void Reset() override
  {
    mfem::IterativeSolverMonitor::Reset();
    _norms.clear();
  }

  void MonitorResidual(int, mfem::real_t norm, const mfem::Vector &, bool) override
  {
    _norms.push_back(norm);
  }

  const std::vector<mfem::real_t> & norms() const { return _norms; }

private:
  std::vector<mfem::real_t> _norms;
};
}

/**
 * Checks that a linear term assembled through the nonlinear form machinery is equivalent to the
 * same term assembled through the bilinear form machinery.
 *
 * Both equation systems discretize the same problem,
 *
 *     -div((u + k) grad u) + u = f   in Omega,      u = g   on boundary 1,
 *
 * and both contain a nonlinear form (the solution-dependent term -div(u grad u)) as well as a
 * bilinear form (the mass term u). They differ only in how the constant-coefficient diffusion
 * term -div(k grad u) is represented:
 *
 *   - the "blf" system uses MFEMDiffusionKernel, a BilinearFormIntegrator;
 *   - the "nlf" system uses MFEMNLDiffusionKernel with a constant k and dk/du = 0, which is a
 *     NonlinearFormIntegrator representing the very same linear operator.
 *
 * The two discrete operators are therefore identical and must produce the same Jacobian and the
 * same Newton residual history.
 *
 * The Dirichlet condition is inhomogeneous in both tests below, so that the comparison covers
 * essential rows carrying nonzero constrained values; the two paths reach those rows through
 * different code - BilinearForm::FormLinearSystem for the bilinear contribution and
 * ParNonlinearForm::GetGradient for the nonlinear one - and a homogeneous condition would not
 * distinguish them. One test uses a uniform condition and a single steady solve; the other uses a
 * condition varying in space and time and re-forms the system at more than one time, as a
 * transient executioner does on each step.
 */
class MFEMNonlinearFormEquivalenceTest : public MFEMObjectUnitTest
{
public:
  MFEMNonlinearFormEquivalenceTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    auto * const pmesh = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
    _fec = std::make_unique<mfem::H1_FECollection>(1, pmesh->Dimension());
    _fespace = std::make_unique<mfem::ParFiniteElementSpace>(pmesh, _fec.get());
    _u = std::make_shared<mfem::ParGridFunction>(_fespace.get());
    *_u = 0.0;
    _mfem_problem->getProblemData().gridfunctions.Register(_var_name, _u);

    auto & coefficients = _mfem_problem->getCoefficients();
    // k(u) = u, so dk/du = 1.
    coefficients.declareScalar<mfem::GridFunctionCoefficient>("k_of_u", _u.get());
    // Boundary value g(x, t). Passing a two-argument function selects mfem::FunctionCoefficient's
    // time-dependent overload, so the value tracks CoefficientManager::setTime().
    coefficients.declareScalar<mfem::FunctionCoefficient>("g_of_x_t", boundaryValue);

    _offsets.SetSize(2);
    _offsets[0] = 0;
    _offsets[1] = _fespace->TrueVSize();

    buildKernels();
  }

  /**
   * Inhomogeneous Dirichlet value, varying over the boundary and with time. All three
   * coordinates appear so that the value is non-constant on any face, whichever one carries the
   * constrained attribute. The mesh spans x in [0, 8] and y, z in [0, 1], and the times used
   * below are O(1), so g stays comfortably positive; the nonlinear diffusivity u + k must not
   * approach zero for the Newton solve to be well behaved.
   */
  static mfem::real_t boundaryValue(const mfem::Vector & p, mfem::real_t t)
  {
    return 2.0 + 0.25 * p(0) + 0.5 * p(1) + 0.75 * p(2) + 0.5 * t;
  }

protected:
  template <typename T>
  std::shared_ptr<T>
  addSharedObject(const std::string & type, const std::string & name, InputParameters & params)
  {
    auto objects = _mfem_problem->addObject<T>(type, name, params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    return objects[0];
  }

  /// Build every weak form component once. Kernels create a fresh integrator on each call to
  /// createBFIntegrator()/createNLIntegrator(), so the shared terms can safely be handed to both
  /// equation systems.
  void buildKernels()
  {
    // Mass term, giving both systems a bilinear form.
    auto mass_params = _factory.getValidParams("MFEMMassKernel");
    mass_params.set<VariableName>("variable") = _var_name;
    mass_params.set<MFEMScalarCoefficientName>("coefficient") = "1.";
    _mass = addSharedObject<MFEMMassKernel>("MFEMMassKernel", "mass", mass_params);

    // Source term, so that the solution is not trivial.
    auto source_params = _factory.getValidParams("MFEMDomainLFKernel");
    source_params.set<VariableName>("variable") = _var_name;
    source_params.set<MFEMScalarCoefficientName>("coefficient") = "1.";
    _source = addSharedObject<MFEMDomainLFKernel>("MFEMDomainLFKernel", "source", source_params);

    // Genuinely nonlinear term, giving both systems a nonlinear form.
    auto nonlinear_params = _factory.getValidParams("MFEMNLDiffusionKernel");
    nonlinear_params.set<VariableName>("variable") = _var_name;
    nonlinear_params.set<MFEMScalarCoefficientName>("k_coefficient") = "k_of_u";
    nonlinear_params.set<MFEMScalarCoefficientName>("dk_du_coefficient") = "1.";
    _nonlinear_diffusion = addSharedObject<MFEMNLDiffusionKernel>(
        "MFEMNLDiffusionKernel", "nl_diff", nonlinear_params);

    // The two inhomogeneous Dirichlet conditions, one uniform and one varying in space and time.
    auto uniform_bc_params = _factory.getValidParams("MFEMScalarDirichletBC");
    uniform_bc_params.set<VariableName>("variable") = _var_name;
    uniform_bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
    uniform_bc_params.set<MFEMScalarCoefficientName>("coefficient") = _uniform_boundary_value;
    _uniform_bc = addSharedObject<MFEMScalarDirichletBC>(
        "MFEMScalarDirichletBC", "uniform_dirichlet", uniform_bc_params);

    auto varying_bc_params = _factory.getValidParams("MFEMScalarDirichletBC");
    varying_bc_params.set<VariableName>("variable") = _var_name;
    varying_bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
    varying_bc_params.set<MFEMScalarCoefficientName>("coefficient") = "g_of_x_t";
    _varying_bc = addSharedObject<MFEMScalarDirichletBC>(
        "MFEMScalarDirichletBC", "varying_dirichlet", varying_bc_params);

    // The term under test, in its two equivalent representations.
    auto blf_params = _factory.getValidParams("MFEMDiffusionKernel");
    blf_params.set<VariableName>("variable") = _var_name;
    blf_params.set<MFEMScalarCoefficientName>("coefficient") = _k_linear;
    _linear_diffusion_blf =
        addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "lin_diff_blf", blf_params);

    auto nlf_params = _factory.getValidParams("MFEMNLDiffusionKernel");
    nlf_params.set<VariableName>("variable") = _var_name;
    nlf_params.set<MFEMScalarCoefficientName>("k_coefficient") = _k_linear;
    nlf_params.set<MFEMScalarCoefficientName>("dk_du_coefficient") = "0.";
    _linear_diffusion_nlf =
        addSharedObject<MFEMNLDiffusionKernel>("MFEMNLDiffusionKernel", "lin_diff_nlf", nlf_params);
  }

  /// Add the terms common to both systems and initialise. Init() may only be called once per
  /// equation system; FormSystem() is what gets repeated per time.
  void buildSystem(Moose::MFEM::EquationSystem & equation_system,
                   std::shared_ptr<MFEMKernel> linear_diffusion,
                   std::shared_ptr<MFEMEssentialBC> dirichlet_bc)
  {
    equation_system.AddKernel(_mass);
    equation_system.AddKernel(_source);
    equation_system.AddKernel(_nonlinear_diffusion);
    equation_system.AddKernel(linear_diffusion);
    equation_system.AddEssentialBC(dirichlet_bc);
    equation_system.Init(_mfem_problem->getProblemData().gridfunctions,
                         _mfem_problem->getProblemData().cmplx_gridfunctions,
                         mfem::AssemblyLevel::LEGACY);
  }

  /// Assemble at the given time and Newton-solve from a zero initial guess.
  /// @returns whether the nonlinear solve converged.
  bool solveAtTime(Moose::MFEM::EquationSystem & equation_system,
                   mfem::real_t time,
                   ResidualHistory & history,
                   mfem::BlockVector & x)
  {
    // Both systems must start from the same iterate and the same boundary data for their
    // histories to be comparable.
    *_u = 0.0;
    _mfem_problem->getCoefficients().setTime(time);

    x.Update(_offsets);
    mfem::BlockVector rhs(_offsets);
    equation_system.FormSystem(x, rhs);

    mfem::GMRESSolver linear_solver(_fespace->GetComm());
    linear_solver.SetKDim(200);
    linear_solver.SetRelTol(1e-14);
    linear_solver.SetAbsTol(1e-16);
    linear_solver.SetMaxIter(1000);
    linear_solver.SetPrintLevel(-1);

    mfem::NewtonSolver newton(_fespace->GetComm());
    newton.iterative_mode = true;
    newton.SetOperator(equation_system);
    newton.SetSolver(linear_solver);
    newton.SetRelTol(_newton_rel_tol);
    newton.SetAbsTol(1e-14);
    newton.SetMaxIter(30);
    newton.SetPrintLevel(-1);
    newton.SetMonitor(history);
    newton.Mult(rhs, x);

    return newton.GetConverged();
  }

  /// Linearize equation_system about x and return the assembled Jacobian. Mult() is called first
  /// because the nonlinear integrators read the current iterate from the trial variable grid
  /// functions rather than from the vector handed to GetGradient(); this mirrors the order
  /// mfem::NewtonSolver uses.
  mfem::HypreParMatrix & jacobianAt(Moose::MFEM::EquationSystem & equation_system,
                                    const mfem::Vector & x)
  {
    mfem::Vector residual(x.Size());
    equation_system.Mult(x, residual);
    return static_cast<mfem::HypreParMatrix &>(equation_system.GetGradient(x));
  }

  /// Largest entry magnitude in a matrix, reduced across all ranks.
  static mfem::real_t maxAbsEntry(const mfem::HypreParMatrix & matrix)
  {
    mfem::SparseMatrix diag, offd;
    HYPRE_BigInt * cmap;
    matrix.GetDiag(diag);
    matrix.GetOffd(offd, cmap);

    mfem::real_t local_max = 0.0;
    for (const auto i : make_range(diag.NumNonZeroElems()))
      local_max = std::max(local_max, std::abs(diag.GetData()[i]));
    for (const auto i : make_range(offd.NumNonZeroElems()))
      local_max = std::max(local_max, std::abs(offd.GetData()[i]));

    mfem::real_t global_max = local_max;
    MPI_Allreduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, matrix.GetComm());
    return global_max;
  }

  /// Global spread of x over the supplied true DoFs, reduced across all ranks.
  mfem::real_t globalRange(const mfem::Vector & x, const mfem::Array<int> & tdofs) const
  {
    mfem::real_t local_min = std::numeric_limits<mfem::real_t>::max();
    mfem::real_t local_max = std::numeric_limits<mfem::real_t>::lowest();
    for (const auto tdof : tdofs)
    {
      local_min = std::min(local_min, x(tdof));
      local_max = std::max(local_max, x(tdof));
    }

    mfem::real_t global_min = local_min, global_max = local_max;
    MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE, MPI_MIN, _fespace->GetComm());
    MPI_Allreduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, _fespace->GetComm());
    return global_max - global_min;
  }

  /// Solve both systems at the supplied time and require that they agree: the same Newton
  /// residual at every iteration, the same solution, the same Jacobian entry by entry, and a unit
  /// diagonal on the essential rows. The bilinear system's solution and the essential true DoFs
  /// are handed back so callers can make further checks on the constrained values.
  void expectEquivalentSolves(Moose::MFEM::EquationSystem & blf_system,
                              Moose::MFEM::EquationSystem & nlf_system,
                              mfem::real_t time,
                              mfem::Vector & blf_solution,
                              mfem::Array<int> & ess_tdofs)
  {
    ResidualHistory blf_history, nlf_history;
    mfem::BlockVector blf_x, nlf_x;

    ASSERT_TRUE(solveAtTime(blf_system, time, blf_history, blf_x));
    ASSERT_TRUE(solveAtTime(nlf_system, time, nlf_history, nlf_x));

    // Both systems must actually exercise both assembly paths, or the comparison is vacuous.
    ASSERT_TRUE(blf_system.IsNonlinear());
    ASSERT_TRUE(nlf_system.IsNonlinear());

    // The problem must be nonlinear enough to need more than one Newton step, otherwise "the same
    // convergence rate" says nothing.
    ASSERT_GT(blf_history.norms().size(), 2u);

    // Same convergence rate: the residual norm at every nonlinear iteration agrees, not merely
    // the final answer. Scaled by the initial norm so the comparison stays meaningful once the
    // residual has dropped to round-off.
    const mfem::real_t history_tol = 1e-10 * blf_history.norms().front();
    ASSERT_EQ(blf_history.norms().size(), nlf_history.norms().size());
    for (const auto i : index_range(blf_history.norms()))
      EXPECT_NEAR(blf_history.norms()[i], nlf_history.norms()[i], history_tol);

    // Same solution.
    mfem::Vector solution_difference(blf_x);
    solution_difference -= nlf_x;
    EXPECT_LT(solution_difference.Norml2(), 1e-10 * blf_x.Norml2());

    // Same Jacobian, compared entry by entry at a common linearization point.
    auto & blf_jacobian = jacobianAt(blf_system, blf_x);
    auto & nlf_jacobian = jacobianAt(nlf_system, blf_x);
    const mfem::real_t jacobian_scale = maxAbsEntry(blf_jacobian);
    ASSERT_GT(jacobian_scale, 0.0);
    std::unique_ptr<mfem::HypreParMatrix> jacobian_difference(
        mfem::Add(1.0, blf_jacobian, -1.0, nlf_jacobian));
    EXPECT_LT(maxAbsEntry(*jacobian_difference), 1e-10 * jacobian_scale);

    // The Jacobian is the gradient of Mult(), which carries a unit diagonal on the essential
    // rows. Both assembly paths impose that constraint independently - the bilinear form through
    // FormLinearSystem's DIAG_ONE policy, the nonlinear form through
    // ParNonlinearForm::GetGradient - so this guards against the two contributions being summed
    // into a diagonal of two.
    _fespace->GetEssentialTrueDofs(blf_system.GetEssentialBoundaryMarkers(_var_name), ess_tdofs);
    ASSERT_GT(ess_tdofs.Size(), 0);
    for (auto * const jacobian : {&blf_jacobian, &nlf_jacobian})
    {
      mfem::Vector jacobian_diagonal;
      jacobian->GetDiag(jacobian_diagonal);
      for (const auto tdof : ess_tdofs)
        EXPECT_DOUBLE_EQ(jacobian_diagonal(tdof), 1.0);
    }

    blf_solution = blf_x;
  }

  const VariableName _var_name{"u"};
  /// Constant diffusivity of the linear term represented two different ways.
  const std::string _k_linear{"2.5"};
  /// Value of the uniform inhomogeneous Dirichlet condition.
  const std::string _uniform_boundary_value{"3.0"};
  /// Loose enough that both solves cross it on the same iteration despite differing by round-off.
  const mfem::real_t _newton_rel_tol{1e-10};
  /// Two well separated times, so that the boundary values differ substantially between them.
  const std::vector<mfem::real_t> _times{0.4, 2.0};
  /// Time used by the steady test, where the condition does not depend on it.
  const mfem::real_t _steady_time{0.0};

  std::unique_ptr<mfem::H1_FECollection> _fec;
  std::unique_ptr<mfem::ParFiniteElementSpace> _fespace;
  std::shared_ptr<mfem::ParGridFunction> _u;
  mfem::Array<int> _offsets;

  std::shared_ptr<MFEMMassKernel> _mass;
  std::shared_ptr<MFEMDomainLFKernel> _source;
  std::shared_ptr<MFEMNLDiffusionKernel> _nonlinear_diffusion;
  std::shared_ptr<MFEMScalarDirichletBC> _uniform_bc;
  std::shared_ptr<MFEMScalarDirichletBC> _varying_bc;
  std::shared_ptr<MFEMDiffusionKernel> _linear_diffusion_blf;
  std::shared_ptr<MFEMNLDiffusionKernel> _linear_diffusion_nlf;
};

/**
 * Steady solve with a uniform, inhomogeneous Dirichlet condition.
 */
TEST_F(MFEMNonlinearFormEquivalenceTest, UniformDirichletCondition)
{
  Moose::MFEM::EquationSystem blf_system;
  buildSystem(blf_system, _linear_diffusion_blf, _uniform_bc);

  Moose::MFEM::EquationSystem nlf_system;
  buildSystem(nlf_system, _linear_diffusion_nlf, _uniform_bc);

  mfem::Vector solution;
  mfem::Array<int> ess_tdofs;
  ASSERT_NO_FATAL_FAILURE(
      expectEquivalentSolves(blf_system, nlf_system, _steady_time, solution, ess_tdofs));

  // A uniform condition constrains every essential DoF to the same nonzero value, so the
  // comparison above covers inhomogeneous essential rows without any spatial variation.
  EXPECT_NEAR(globalRange(solution, ess_tdofs), 0.0, 1e-10);
  for (const auto tdof : ess_tdofs)
    EXPECT_NEAR(solution(tdof), std::stod(_uniform_boundary_value), 1e-10);
}

/**
 * Repeated solves with a Dirichlet condition varying in space and time.
 */
TEST_F(MFEMNonlinearFormEquivalenceTest, SpaceAndTimeDependentDirichletCondition)
{
  Moose::MFEM::EquationSystem blf_system;
  buildSystem(blf_system, _linear_diffusion_blf, _varying_bc);

  Moose::MFEM::EquationSystem nlf_system;
  buildSystem(nlf_system, _linear_diffusion_nlf, _varying_bc);

  mfem::Array<int> ess_tdofs;
  std::vector<mfem::Vector> solutions_by_time;

  for (const auto time : _times)
  {
    SCOPED_TRACE("time = " + std::to_string(time));

    mfem::Vector solution;
    ASSERT_NO_FATAL_FAILURE(
        expectEquivalentSolves(blf_system, nlf_system, time, solution, ess_tdofs));

    // The constrained values must genuinely vary over the boundary, or the spatial dependence of
    // the condition never reaches the essential rows being compared. Boundary 1 is a face of
    // constant x spanning the unit square in y and z, so g varies by 0.5 + 0.75 across it.
    EXPECT_NEAR(globalRange(solution, ess_tdofs), 1.25, 1e-10);

    solutions_by_time.push_back(solution);
  }

  // The constrained values must also change with time, or re-forming the system at a second time
  // adds nothing. Time enters g as a uniform offset, so the spread over the boundary is unchanged
  // while the solution itself is not.
  ASSERT_EQ(solutions_by_time.size(), _times.size());
  mfem::Vector time_difference(solutions_by_time.front());
  time_difference -= solutions_by_time.back();
  EXPECT_GT(time_difference.Norml2(), 0.1 * solutions_by_time.front().Norml2());
}

#endif
