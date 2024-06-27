#include "div_free_source.hpp"
#include "helmholtz_projector.hpp"

namespace hephaestus
{

/*
Returns the Helmholtz (divergence-free) projection P(g) of a source g
defined by
(∇.P(g), q) = 0
where
P(g) = g - ∇Q         Q ∈ H1
P(g) = g + β∇p - ∇×M

via the weak form:
(g, ∇q) - (∇Q, ∇q) - <P(g).n, q> = 0
*/
DivFreeSource::DivFreeSource(std::string src_coef_name,
                             std::string src_gf_name,
                             std::string hcurl_fespace_name,
                             std::string h1_fespace_name,
                             std::string potential_gf_name,
                             hephaestus::InputParameters solver_options,
                             bool perform_helmholtz_projection)
  : _src_coef_name(std::move(src_coef_name)),
    _src_gf_name(std::move(src_gf_name)),
    _hcurl_fespace_name(std::move(hcurl_fespace_name)),
    _h1_fespace_name(std::move(h1_fespace_name)),
    _potential_gf_name(std::move(potential_gf_name)),
    _solver_options(std::move(solver_options)),
    _perform_helmholtz_projection(std::move(perform_helmholtz_projection)),
    _h_curl_mass(nullptr)
{
}

void
DivFreeSource::Init(hephaestus::GridFunctions & gridfunctions,
                    const hephaestus::FESpaces & fespaces,
                    hephaestus::BCMap & bc_map,
                    hephaestus::Coefficients & coefficients)
{
  _h1_fe_space = fespaces.Get(_h1_fespace_name);
  _h_curl_fe_space = fespaces.Get(_hcurl_fespace_name);
  _source_vec_coef = coefficients._vectors.Get(_src_coef_name);

  _div_free_src_gf = std::make_shared<mfem::ParGridFunction>(_h_curl_fe_space);
  gridfunctions.Register(_src_gf_name, _div_free_src_gf);

  _g = std::make_shared<mfem::ParGridFunction>(_h_curl_fe_space);
  gridfunctions.Register("_user_source", _g);

  _q = std::make_shared<mfem::ParGridFunction>(_h1_fe_space);
  gridfunctions.Register(_potential_gf_name, _q);

  _bc_map = &bc_map;
  _gridfunctions = &gridfunctions;
  _fespaces = &fespaces;

  BuildHCurlMass();
}

void
DivFreeSource::BuildHCurlMass()
{
  _h_curl_mass = std::make_unique<mfem::ParBilinearForm>(_h_curl_fe_space);
  _h_curl_mass->AddDomainIntegrator(new mfem::VectorFEMassIntegrator);
  _h_curl_mass->Assemble();
  _h_curl_mass->Finalize();
}

void
DivFreeSource::Apply(mfem::ParLinearForm * lf)
{
  // Find an averaged representation of current density in H(curl)*
  _g->ProjectCoefficient(*_source_vec_coef);
  mfem::ParLinearForm j(_h_curl_fe_space);
  j.AddDomainIntegrator(new mfem::VectorFEDomainLFIntegrator(*_source_vec_coef));
  j.Assemble();
  {
    mfem::HypreParMatrix m;
    mfem::Vector x, rhs;
    mfem::Array<int> ess_tdof_list;
    _h_curl_mass->FormLinearSystem(ess_tdof_list, *_g, j, m, x, rhs);

    DefaultGMRESSolver solver(_solver_options, m);
    solver.Mult(rhs, x);

    _h_curl_mass->RecoverFEMSolution(x, j, *_g);
  }

  *_div_free_src_gf = *_g;

  if (_perform_helmholtz_projection)
  {

    hephaestus::InputParameters projector_pars;
    projector_pars.SetParam("VectorGridFunctionName", _src_gf_name);
    projector_pars.SetParam("ScalarGridFunctionName", _potential_gf_name);
    projector_pars.SetParam("H1FESpaceName", _h1_fespace_name);
    projector_pars.SetParam("HCurlFESpaceName", _hcurl_fespace_name);

    hephaestus::BCMap bcs;

    hephaestus::HelmholtzProjector projector(projector_pars);
    projector.Project(*_gridfunctions, *_fespaces, bcs);
  }

  // Add divergence free source to target linear form
  _h_curl_mass->Update();
  _h_curl_mass->Assemble();
  _h_curl_mass->Finalize();

  _h_curl_mass->AddMult(*_div_free_src_gf, *lf, 1.0);
}

void
DivFreeSource::SubtractSource(mfem::ParGridFunction * gf)
{
  _h_curl_mass->AddMult(*_div_free_src_gf, *gf, -1.0);
}

} // namespace hephaestus
