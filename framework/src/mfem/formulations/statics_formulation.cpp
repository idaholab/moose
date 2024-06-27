// Solves the equations
// ∇⋅s0 = 0
// ∇×(α∇×u) = s0

// where
// s0 ∈ H(div) source field
// u ∈ H(curl)

// Dirichlet boundaries constrain u
// Integrated boundaries constrain (α∇×u) × n

// Weak form (Space discretisation)
// (α∇×u, ∇×u') - (s0, u') - <(α∇×u) × n, u'> = 0

// Time discretisation using implicit scheme:
// Unknowns
// u ∈ H(curl)

// Fully discretised equations
// (α∇×u, ∇×u') - (s0, u') - <(α∇×u) × n, u'> = 0

// Rewritten as
// a1(u, u') = b1(u')

// where
// a1(u, u') = (α∇×u, ∇×u')
// b1(u') = (s0, u') + <(α∇×u) × n, u'>
#include "statics_formulation.hpp"

#include <utility>

namespace hephaestus
{

StaticsFormulation::StaticsFormulation(std::string alpha_coef_name, std::string h_curl_var_name)
  : _alpha_coef_name(std::move(alpha_coef_name)), _h_curl_var_name(std::move(h_curl_var_name))
{
}

void
StaticsFormulation::ConstructJacobianPreconditioner()
{
  std::shared_ptr<mfem::HypreAMS> precond{std::make_shared<mfem::HypreAMS>(
      GetProblem()->_gridfunctions.Get(_h_curl_var_name)->ParFESpace())};

  precond->SetSingularProblem();
  precond->SetPrintLevel(-1);

  GetProblem()->_jacobian_preconditioner = precond;
}

void
StaticsFormulation::ConstructJacobianSolver()
{
  ConstructJacobianSolverWithOptions(SolverType::HYPRE_FGMRES,
                                     {._max_iteration = 100, ._k_dim = 10});
}

void
StaticsFormulation::ConstructOperator()
{
  auto new_operator = std::make_unique<hephaestus::StaticsOperator>(
      *GetProblem(), _h_curl_var_name, _alpha_coef_name);

  GetProblem()->SetOperator(std::move(new_operator));
}

void
StaticsFormulation::RegisterGridFunctions()
{
  int & myid = GetProblem()->_myid;
  hephaestus::GridFunctions & gridfunctions = GetProblem()->_gridfunctions;
  hephaestus::FESpaces & fespaces = GetProblem()->_fespaces;

  // Register default ParGridFunctions of state gridfunctions if not provided
  if (!gridfunctions.Has(_h_curl_var_name))
  {
    if (myid == 0)
    {
      MFEM_WARNING(_h_curl_var_name << " not found in gridfunctions: building "
                                       "gridfunction from defaults");
    }
    AddFESpace(std::string("_HCurlFESpace"), std::string("ND_3D_P2"));
    AddGridFunction(_h_curl_var_name, std::string("_HCurlFESpace"));
  };
};

void
StaticsFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;
  if (!coefficients._scalars.Has(_alpha_coef_name))
  {
    MFEM_ABORT(_alpha_coef_name + " coefficient not found.");
  }
}

StaticsOperator::StaticsOperator(hephaestus::Problem & problem,
                                 std::string h_curl_var_name,
                                 std::string stiffness_coef_name)
  : ProblemOperator(problem),
    _h_curl_var_name(std::move(h_curl_var_name)),
    _stiffness_coef_name(std::move(stiffness_coef_name))
{
}

void
StaticsOperator::SetGridFunctions()
{
  _trial_var_names.push_back(_h_curl_var_name);
  ProblemOperator::SetGridFunctions();
};

void
StaticsOperator::Init(mfem::Vector & X)
{
  ProblemOperator::Init(X);
  _stiff_coef = _problem._coefficients._scalars.Get(_stiffness_coef_name);
}

/*
This is the main method that solves for u.

Unknowns
s0 ∈ H(div) divergence-free source field
u ∈ H(curl)

Fully discretised equations
(α∇×u, ∇×u') - (s0, u') - <(α∇×u) × n, u'> = 0
*/
void
StaticsOperator::Solve(mfem::Vector & X)
{
  spdlog::stopwatch sw;

  mfem::ParGridFunction & gf(*_trial_variables.at(0));
  gf = 0.0;
  mfem::ParLinearForm lf(gf.ParFESpace());
  lf = 0.0;
  mfem::Array<int> ess_bdr_tdofs;
  _problem._bc_map.ApplyEssentialBCs(_h_curl_var_name, ess_bdr_tdofs, gf, _problem._pmesh.get());
  _problem._bc_map.ApplyIntegratedBCs(_h_curl_var_name, lf, _problem._pmesh.get());
  lf.Assemble();
  _problem._sources.Apply(&lf);
  mfem::ParBilinearForm blf(gf.ParFESpace());
  blf.AddDomainIntegrator(new mfem::CurlCurlIntegrator(*_stiff_coef));
  blf.Assemble();
  blf.Finalize();
  mfem::HypreParMatrix curl_mu_inv_curl;
  mfem::HypreParVector sol_tdofs(gf.ParFESpace());
  mfem::HypreParVector rhs_tdofs(gf.ParFESpace());
  blf.FormLinearSystem(ess_bdr_tdofs, gf, lf, curl_mu_inv_curl, sol_tdofs, rhs_tdofs);

  // Define and apply a parallel FGMRES solver for AX=B with the AMS
  // preconditioner from hypre.
  _problem._jacobian_solver->SetOperator(curl_mu_inv_curl);
  _problem._jacobian_solver->Mult(rhs_tdofs, sol_tdofs);
  blf.RecoverFEMSolution(sol_tdofs, lf, gf);

  logger.info("{} Solve: {} seconds", typeid(this).name(), sw);
}

} // namespace hephaestus
