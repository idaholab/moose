#include "diffusion_kernel.h"

namespace platypus
{

DiffusionKernel::DiffusionKernel(const platypus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
DiffusionKernel::Init(platypus::GridFunctions & gridfunctions,
                      const platypus::FESpaces & fespaces,
                      platypus::BCMap & bc_map,
                      Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
DiffusionKernel::Apply(mfem::ParBilinearForm * blf)
{
  blf->AddDomainIntegrator(new mfem::DiffusionIntegrator(*_coef));
}

} // namespace platypus
