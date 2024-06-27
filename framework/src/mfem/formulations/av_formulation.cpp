// Solves:
// ∇×(ν∇×A) + σ(dA/dt + ∇ V) = Jᵉ
// ∇·(σ(dA/dt + ∇ V))= 0

// where
// Jᵉ ∈ H(div) source field
// A ∈ H(curl)
// V ∈ H1

//* in weak form
//* (ν∇×A, ∇×A') + (σ(dA/dt + ∇ V), A') - (Jᵉ, A') - <(ν∇×A) × n, A'>  = 0
//* (σ(dA/dt + ∇ V), ∇V') - <σ(dA/dt + ∇ V)·n, V'> =0
//*
//* where:
//* reluctivity ν = 1/μ
//* electrical_conductivity σ=1/ρ
//* Magnetic vector potential A
//* Scalar electric potential V
//* Electric field, E = -dA/dt -∇V
//* Magnetic flux density, B = ∇×A
//* Magnetic field H = ν∇×A
//* Current density J = -σ(dA/dt + ∇ V)

//* Either:
//* B.n (or E×n) at boundary: A×n (Dirichlet)
//* H×n at boundary: ν∇×A (Integrated)
//* -σ(dA/dt + ∇ V)·n (J·n, Neumann), V (potential, Dirichlet)

#include "av_formulation.hpp"

#include <utility>

namespace hephaestus
{

AVFormulation::AVFormulation(std::string alpha_coef_name,
                             std::string inv_alpha_coef_name,
                             std::string beta_coef_name,
                             std::string vector_potential_name,
                             std::string scalar_potential_name)
  : _alpha_coef_name(std::move(alpha_coef_name)),
    _inv_alpha_coef_name(std::move(inv_alpha_coef_name)),
    _beta_coef_name(std::move(beta_coef_name)),
    _vector_potential_name(std::move(vector_potential_name)),
    _scalar_potential_name(std::move(scalar_potential_name))
{
}

void
AVFormulation::ConstructOperator()
{
  hephaestus::InputParameters av_system_params;
  av_system_params.SetParam("VectorPotentialName", _vector_potential_name);
  av_system_params.SetParam("ScalarPotentialName", _scalar_potential_name);
  av_system_params.SetParam("AlphaCoefName", _alpha_coef_name);
  av_system_params.SetParam("BetaCoefName", _beta_coef_name);

  auto equation_system = std::make_unique<hephaestus::AVEquationSystem>(av_system_params);

  GetProblem()->SetOperator(std::make_unique<hephaestus::TimeDomainEquationSystemProblemOperator>(
      *GetProblem(), std::move(equation_system)));
}

void
AVFormulation::RegisterGridFunctions()
{
  int & myid = GetProblem()->_myid;
  hephaestus::GridFunctions & gridfunctions = GetProblem()->_gridfunctions;

  // Register default ParGridFunctions of state gridfunctions if not provided
  if (!gridfunctions.Has(_vector_potential_name))
  {
    if (myid == 0)
    {
      MFEM_WARNING(_vector_potential_name
                   << " not found in gridfunctions: building gridfunction from "
                      "defaults");
    }
    AddFESpace(std::string("_HCurlFESpace"), std::string("ND_3D_P2"));
    AddGridFunction(_vector_potential_name, std::string("_HCurlFESpace"));
  }

  // Register default ParGridFunctions of state gridfunctions if not provided
  if (!gridfunctions.Has(_scalar_potential_name))
  {
    if (myid == 0)
    {
      MFEM_WARNING(_scalar_potential_name
                   << " not found in gridfunctions: building gridfunction from "
                      "defaults");
    }
    AddFESpace(std::string("_H1FESpace"), std::string("H1_3D_P2"));
    AddGridFunction(_scalar_potential_name, std::string("_H1FESpace"));
  }

  // Register time derivatives
  TimeDomainEquationSystemProblemBuilder::RegisterGridFunctions();
};

void
AVFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;
  if (!coefficients._scalars.Has(_inv_alpha_coef_name))
  {
    MFEM_ABORT(_inv_alpha_coef_name + " coefficient not found.");
  }
  if (!coefficients._scalars.Has(_beta_coef_name))
  {
    MFEM_ABORT(_beta_coef_name + " coefficient not found.");
  }

  coefficients._scalars.Register(
      _alpha_coef_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_one_coef, coefficients._scalars.Get(_inv_alpha_coef_name), fracFunc));
}

AVEquationSystem::AVEquationSystem(const hephaestus::InputParameters & params)
  : _a_name(params.GetParam<std::string>("VectorPotentialName")),
    _v_name(params.GetParam<std::string>("ScalarPotentialName")),
    _alpha_coef_name(params.GetParam<std::string>("AlphaCoefName")),
    _beta_coef_name(params.GetParam<std::string>("BetaCoefName")),
    _dtalpha_coef_name(std::string("dt_") + _alpha_coef_name),
    _neg_beta_coef_name(std::string("negative_") + _beta_coef_name),
    _neg_coef(-1.0)
{
}

void
AVEquationSystem::Init(hephaestus::GridFunctions & gridfunctions,
                       const hephaestus::FESpaces & fespaces,
                       hephaestus::BCMap & bc_map,
                       hephaestus::Coefficients & coefficients)
{
  coefficients._scalars.Register(
      _dtalpha_coef_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_dt_coef, coefficients._scalars.Get(_alpha_coef_name), prodFunc));

  coefficients._scalars.Register(
      _neg_beta_coef_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_neg_coef, coefficients._scalars.Get(_beta_coef_name), prodFunc));

  TimeDependentEquationSystem::Init(gridfunctions, fespaces, bc_map, coefficients);
}

void
AVEquationSystem::AddKernels()
{
  spdlog::stopwatch sw;

  AddTrialVariableNameIfMissing(_a_name);
  std::string da_dt_name = GetTimeDerivativeName(_a_name);
  AddTrialVariableNameIfMissing(_v_name);
  std::string dv_dt_name = GetTimeDerivativeName(_v_name);

  // (α∇×A_{n}, ∇×dA'/dt)
  hephaestus::InputParameters weak_curl_curl_params;
  weak_curl_curl_params.SetParam("CoupledVariableName", _a_name);
  weak_curl_curl_params.SetParam("CoefficientName", _alpha_coef_name);
  AddKernel(da_dt_name, std::make_shared<hephaestus::WeakCurlCurlKernel>(weak_curl_curl_params));

  // (αdt∇×dA/dt_{n+1}, ∇×dA'/dt)
  hephaestus::InputParameters curl_curl_params;
  curl_curl_params.SetParam("CoefficientName", _dtalpha_coef_name);
  AddKernel(da_dt_name, std::make_shared<hephaestus::CurlCurlKernel>(curl_curl_params));

  // (βdA/dt_{n+1}, dA'/dt)
  hephaestus::InputParameters vector_fe_mass_params;
  vector_fe_mass_params.SetParam("CoefficientName", _beta_coef_name);
  AddKernel(da_dt_name, std::make_shared<hephaestus::VectorFEMassKernel>(vector_fe_mass_params));

  // (σ ∇ V, dA'/dt)
  hephaestus::InputParameters mixed_vector_gradient_params;
  mixed_vector_gradient_params.SetParam("CoefficientName", _beta_coef_name);
  AddKernel(_v_name,
            da_dt_name,
            std::make_shared<hephaestus::MixedVectorGradientKernel>(mixed_vector_gradient_params));

  // (σ ∇ V, ∇ V')
  hephaestus::InputParameters diffusion_params;
  diffusion_params.SetParam("CoefficientName", _beta_coef_name);
  AddKernel(_v_name, std::make_shared<hephaestus::DiffusionKernel>(diffusion_params));

  // (σdA/dt, ∇ V')
  hephaestus::InputParameters vector_fe_weak_divergence_params;
  vector_fe_weak_divergence_params.SetParam("CoefficientName", _beta_coef_name);
  AddKernel(
      da_dt_name,
      _v_name,
      std::make_shared<hephaestus::VectorFEWeakDivergenceKernel>(vector_fe_weak_divergence_params));

  logger.info("{} AddKernels: {} seconds", typeid(this).name(), sw);
}

} // namespace hephaestus
