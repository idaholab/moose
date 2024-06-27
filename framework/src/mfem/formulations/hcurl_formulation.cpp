// Solves the equations
// ∇⋅s0 = 0
// ∇×(α∇×u) + βdu/dt = s0

// where
// s0 ∈ H(div) source field
// u ∈ H(curl)
// p ∈ H1

// Dirichlet boundaries constrain du/dt
// Integrated boundaries constrain (α∇×u) × n

// Weak form (Space discretisation)
// -(s0, ∇ p') + <n.s0, p'> = 0
// (α∇×u, ∇×u') + (βdu/dt, u') - (s0, u') - <(α∇×u) × n, u'> = 0

// Time discretisation using implicit scheme:
// Unknowns
// s0_{n+1} ∈ H(div) source field, where s0 = -β∇p
// du/dt_{n+1} ∈ H(curl)
// p_{n+1} ∈ H1

// Fully discretised equations
// -(s0_{n+1}, ∇ p') + <n.s0_{n+1}, p'> = 0
// (α∇×u_{n}, ∇×u') + (αdt∇×du/dt_{n+1}, ∇×u') + (βdu/dt_{n+1}, u')
// - (s0_{n+1}, u') - <(α∇×u_{n+1}) × n, u'> = 0
// using
// u_{n+1} = u_{n} + dt du/dt_{n+1}

// Rewritten as
// a0(p_{n+1}, p') = b0(p')
// a1(du/dt_{n+1}, u') = b1(u')

// where
// a0(p, p') = (β ∇ p, ∇ p')
// b0(p') = <n.s0, p'>
// a1(u, u') = (βu, u') + (αdt∇×u, ∇×u')
// b1(u') = (s0_{n+1}, u') - (α∇×u_{n}, ∇×u') + <(α∇×u_{n+1}) × n, u'>
#include "hcurl_formulation.hpp"

#include <utility>

namespace hephaestus
{

HCurlFormulation::HCurlFormulation(std::string alpha_coef_name,
                                   std::string beta_coef_name,
                                   std::string h_curl_var_name)
  : _alpha_coef_name(std::move(alpha_coef_name)),
    _beta_coef_name(std::move(beta_coef_name)),
    _h_curl_var_name(std::move(h_curl_var_name))
{
}

void
HCurlFormulation::ConstructOperator()
{
  hephaestus::InputParameters weak_form_params;
  weak_form_params.SetParam("HCurlVarName", _h_curl_var_name);
  weak_form_params.SetParam("AlphaCoefName", _alpha_coef_name);
  weak_form_params.SetParam("BetaCoefName", _beta_coef_name);

  auto equation_system = std::make_unique<hephaestus::CurlCurlEquationSystem>(weak_form_params);

  GetProblem()->SetOperator(std::make_unique<hephaestus::TimeDomainEquationSystemProblemOperator>(
      *GetProblem(), std::move(equation_system)));
}

void
HCurlFormulation::ConstructJacobianPreconditioner()
{
  auto precond =
      std::make_shared<mfem::HypreAMS>(GetProblem()->GetEquationSystem()->_test_pfespaces.at(0));

  precond->SetSingularProblem();
  precond->SetPrintLevel(-1);

  GetProblem()->_jacobian_preconditioner = precond;
}

void
HCurlFormulation::ConstructJacobianSolver()
{
  ConstructJacobianSolverWithOptions(SolverType::HYPRE_PCG);
}

void
HCurlFormulation::RegisterGridFunctions()
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
  // Register time derivatives
  TimeDomainEquationSystemProblemBuilder::RegisterGridFunctions();
};

CurlCurlEquationSystem::CurlCurlEquationSystem(const hephaestus::InputParameters & params)
  : _h_curl_var_name(params.GetParam<std::string>("HCurlVarName")),
    _alpha_coef_name(params.GetParam<std::string>("AlphaCoefName")),
    _beta_coef_name(params.GetParam<std::string>("BetaCoefName")),
    _dtalpha_coef_name(std::string("dt_") + _alpha_coef_name)
{
}

void
CurlCurlEquationSystem::Init(hephaestus::GridFunctions & gridfunctions,
                             const hephaestus::FESpaces & fespaces,
                             hephaestus::BCMap & bc_map,
                             hephaestus::Coefficients & coefficients)
{
  coefficients._scalars.Register(
      _dtalpha_coef_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_dt_coef, coefficients._scalars.Get(_alpha_coef_name), prodFunc));
  TimeDependentEquationSystem::Init(gridfunctions, fespaces, bc_map, coefficients);
}

void
CurlCurlEquationSystem::AddKernels()
{
  spdlog::stopwatch sw;

  AddTrialVariableNameIfMissing(_h_curl_var_name);
  std::string dh_curl_var_dt = GetTimeDerivativeName(_h_curl_var_name);

  // (α∇×u_{n}, ∇×u')
  hephaestus::InputParameters weak_curl_curl_params;
  weak_curl_curl_params.SetParam("CoupledVariableName", _h_curl_var_name);
  weak_curl_curl_params.SetParam("CoefficientName", _alpha_coef_name);
  AddKernel(dh_curl_var_dt,
            std::make_shared<hephaestus::WeakCurlCurlKernel>(weak_curl_curl_params));

  // (αdt∇×du/dt_{n+1}, ∇×u')
  hephaestus::InputParameters curl_curl_params;
  curl_curl_params.SetParam("CoefficientName", _dtalpha_coef_name);
  AddKernel(dh_curl_var_dt, std::make_shared<hephaestus::CurlCurlKernel>(curl_curl_params));

  // (βdu/dt_{n+1}, u')
  hephaestus::InputParameters vector_fe_mass_params;
  vector_fe_mass_params.SetParam("CoefficientName", _beta_coef_name);
  AddKernel(dh_curl_var_dt,
            std::make_shared<hephaestus::VectorFEMassKernel>(vector_fe_mass_params));

  logger.info("{} AddKernels: {} seconds", typeid(this).name(), sw);
}

void
HCurlFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;
  if (!coefficients._scalars.Has(_alpha_coef_name))
  {
    MFEM_ABORT(_alpha_coef_name + " coefficient not found.");
  }
  if (!coefficients._scalars.Has(_beta_coef_name))
  {
    MFEM_ABORT(_beta_coef_name + " coefficient not found.");
  }
}

} // namespace hephaestus
