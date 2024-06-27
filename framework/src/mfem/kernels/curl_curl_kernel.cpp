#include "curl_curl_kernel.hpp"

namespace hephaestus
{

CurlCurlKernel::CurlCurlKernel(const hephaestus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
CurlCurlKernel::Init(hephaestus::GridFunctions & gridfunctions,
                     const hephaestus::FESpaces & fespaces,
                     hephaestus::BCMap & bc_map,
                     hephaestus::Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
CurlCurlKernel::Apply(mfem::ParBilinearForm * blf)
{
  blf->AddDomainIntegrator(new mfem::CurlCurlIntegrator(*_coef));
}

} // namespace hephaestus
