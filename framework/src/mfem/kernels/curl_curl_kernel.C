#include "curl_curl_kernel.h"

namespace platypus
{

CurlCurlKernel::CurlCurlKernel(const platypus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
CurlCurlKernel::Init(platypus::GridFunctions & gridfunctions,
                     const platypus::FESpaces & fespaces,
                     platypus::BCMap & bc_map,
                     Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
CurlCurlKernel::Apply(mfem::ParBilinearForm * blf)
{
  blf->AddDomainIntegrator(new mfem::CurlCurlIntegrator(*_coef));
}

} // namespace platypus
