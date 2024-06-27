#include "weak_curl_curl_kernel.hpp"

namespace hephaestus
{

WeakCurlCurlKernel::WeakCurlCurlKernel(const hephaestus::InputParameters & params)
  : Kernel(params),
    _coupled_gf_name(params.GetParam<std::string>("CoupledVariableName")),
    _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
WeakCurlCurlKernel::Init(hephaestus::GridFunctions & gridfunctions,
                         const hephaestus::FESpaces & fespaces,
                         hephaestus::BCMap & bc_map,
                         hephaestus::Coefficients & coefficients)
{
  _u = gridfunctions.Get(_coupled_gf_name);
  _coef = coefficients._scalars.Get(_coef_name);

  _curl_curl = std::make_unique<mfem::ParBilinearForm>(_u->ParFESpace());
  _curl_curl->AddDomainIntegrator(new mfem::CurlCurlIntegrator(*_coef));
  _curl_curl->Assemble();
}

void
WeakCurlCurlKernel::Apply(mfem::ParLinearForm * lf)
{
  _curl_curl->AddMultTranspose(*_u, *lf, -1.0);
}

} // namespace hephaestus
