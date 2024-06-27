#include "helmholtz_projector.hpp"

namespace hephaestus
{

HelmholtzProjector::HelmholtzProjector(const hephaestus::InputParameters & params)
  : _h1_fespace_name(params.GetOptionalParam<std::string>("H1FESpaceName", "H1FES_Name")),
    _hcurl_fespace_name(params.GetOptionalParam<std::string>("HCurlFESpaceName", "HCurlFES_Name")),
    _gf_grad_name(params.GetParam<std::string>("VectorGridFunctionName")),
    _gf_name(params.GetOptionalParam<std::string>("ScalarGridFunctionName", "ScalarGF_Name")),

    _g(nullptr),

    _g_div(nullptr),
    _weak_div(nullptr),
    _grad(nullptr),
    _a0(nullptr)
{

  hephaestus::InputParameters default_pars;
  default_pars.SetParam("Tolerance", float(1.0e-20));
  default_pars.SetParam("AbsTolerance", float(1.0e-20));
  default_pars.SetParam("MaxIter", (unsigned int)1000);
  default_pars.SetParam("PrintLevel", GetGlobalPrintLevel());

  _solver_options =
      params.GetOptionalParam<hephaestus::InputParameters>("SolverOptions", default_pars);
}

void
HelmholtzProjector::Project(hephaestus::GridFunctions & gridfunctions,
                            const hephaestus::FESpaces & fespaces,
                            hephaestus::BCMap & bc_map)
{

  // Retrieving vector GridFunction. This is the only mandatory one
  _div_free_src_gf = gridfunctions.Get(_gf_grad_name);

  if (!fespaces.Has(_hcurl_fespace_name))
  {
    logger.info("{} not found in fespaces when creating {}. Obtaining from vector "
                "GridFunction.",
                _hcurl_fespace_name,
                typeid(this).name());
    _h_curl_fe_space = _div_free_src_gf->ParFESpace();
  }
  else
  {
    _h_curl_fe_space = fespaces.Get(_hcurl_fespace_name);
  }

  std::unique_ptr<mfem::H1_FECollection> h1_fec{nullptr};

  if (!fespaces.Has(_h1_fespace_name))
  {
    logger.info("{} not found in fespaces when creating {}. Extracting from GridFunction",
                _h1_fespace_name,
                typeid(this).name());

    // Creates an H1 FES on the same mesh and with the same order as the HCurl
    // FES
    h1_fec = std::make_unique<mfem::H1_FECollection>(_h_curl_fe_space->GetMaxElementOrder(),
                                                     _h_curl_fe_space->GetParMesh()->Dimension());

    _h1_fe_space =
        std::make_shared<mfem::ParFiniteElementSpace>(_h_curl_fe_space->GetParMesh(), h1_fec.get());
  }
  else
  {
    _h1_fe_space = fespaces.GetShared(_h1_fespace_name);
  }

  if (!gridfunctions.Has(_gf_name))
  {
    logger.info("{} not found in gridfunctions when creating {}. Creating new GridFunction",
                _gf_name,
                typeid(this).name());
    _q = std::make_shared<mfem::ParGridFunction>(_h1_fe_space.get());
  }
  else
  {
    _q = gridfunctions.GetShared(_gf_name);
  }

  _g = std::make_unique<mfem::ParGridFunction>(_h_curl_fe_space);
  *_g = *_div_free_src_gf;
  *_q = 0.0;

  _bc_map = &bc_map;

  SetForms();
  SetGrad();
  SetBCs();
  SolveLinearSystem();

  // Compute the irrotational component of g
  // P(g) = g - ∇Q
  _grad->Mult(*_q, *_div_free_src_gf);
  *_div_free_src_gf -= *_g;
  *_div_free_src_gf *= -1.0;
}

void
HelmholtzProjector::SetForms()
{
  if (_g_div == nullptr)
    _g_div = std::make_unique<mfem::ParLinearForm>(_h1_fe_space.get());

  if (_weak_div == nullptr)
  {
    _weak_div = std::make_unique<mfem::ParMixedBilinearForm>(_h_curl_fe_space, _h1_fe_space.get());
    _weak_div->AddDomainIntegrator(new mfem::VectorFEWeakDivergenceIntegrator);
    _weak_div->Assemble();
    _weak_div->Finalize();
  }

  if (_a0 == nullptr)
  {
    _a0 = std::make_unique<mfem::ParBilinearForm>(_h1_fe_space.get());
    _a0->AddDomainIntegrator(new mfem::DiffusionIntegrator);
    _a0->Assemble();
    _a0->Finalize();
  }
}

void
HelmholtzProjector::SetGrad()
{
  if (_grad == nullptr)
  {
    _grad = std::make_unique<mfem::ParDiscreteLinearOperator>(_h1_fe_space.get(), _h_curl_fe_space);
    _grad->AddDomainInterpolator(new mfem::GradientInterpolator());
    _grad->Assemble();
    _grad->Finalize();
  }
}

void
HelmholtzProjector::SetBCs()
{
  // Begin Divergence-free projection
  // (g, ∇q) - (∇Q, ∇q) - <P(g).n, q> = 0
  int myid = _h1_fe_space->GetMyRank();

  // <P(g).n, q>
  _bc_map->ApplyEssentialBCs(_gf_name, _ess_bdr_tdofs, *_q, (_h1_fe_space->GetParMesh()));
  _bc_map->ApplyIntegratedBCs(_gf_name, *_g_div, (_h1_fe_space->GetParMesh()));

  // Apply essential BC. Necessary to ensure potential at least one point is
  // fixed.
  int localsize = _ess_bdr_tdofs.Size();
  int fullsize;
  MPI_Allreduce(&fullsize, &localsize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  if (fullsize == 0 && myid == 0)
  {
    _ess_bdr_tdofs.SetSize(1);
    _ess_bdr_tdofs[0] = 0;
  }
}

void
HelmholtzProjector::SolveLinearSystem()
{
  _g_div->Assemble();

  // Compute the divergence of g
  // (g, ∇q)
  _weak_div->AddMult(*_g, *_g_div, -1.0);

  // Form linear system
  // (g, ∇q) - (∇Q, ∇q) - <P(g).n, q> = 0
  // (∇Q, ∇q) = (g, ∇q) - <P(g).n, q>
  mfem::HypreParMatrix a0;
  mfem::Vector x0;
  mfem::Vector b0;
  _a0->FormLinearSystem(_ess_bdr_tdofs, *_q, *_g_div, a0, x0, b0);

  hephaestus::DefaultGMRESSolver a0_solver(_solver_options, a0);

  a0_solver.Mult(b0, x0);
  _a0->RecoverFEMSolution(x0, *_g_div, *_q);
}

} // namespace hephaestus