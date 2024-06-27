#include "mixed_vector_gradient_kernel.h"

namespace platypus
{

MixedVectorGradientKernel::MixedVectorGradientKernel(const platypus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
MixedVectorGradientKernel::Init(platypus::GridFunctions & gridfunctions,
                                const platypus::FESpaces & fespaces,
                                platypus::BCMap & bc_map,
                                Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
MixedVectorGradientKernel::Apply(mfem::ParMixedBilinearForm * mblf)
{
  mblf->AddDomainIntegrator(new mfem::MixedVectorGradientIntegrator(*_coef));
}

} // namespace platypus
