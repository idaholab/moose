#include "mixed_vector_gradient_kernel.hpp"

namespace hephaestus
{

MixedVectorGradientKernel::MixedVectorGradientKernel(const hephaestus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
MixedVectorGradientKernel::Init(hephaestus::GridFunctions & gridfunctions,
                                const hephaestus::FESpaces & fespaces,
                                hephaestus::BCMap & bc_map,
                                hephaestus::Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
MixedVectorGradientKernel::Apply(mfem::ParMixedBilinearForm * mblf)
{
  mblf->AddDomainIntegrator(new mfem::MixedVectorGradientIntegrator(*_coef));
}

} // namespace hephaestus
