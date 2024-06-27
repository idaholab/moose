#include "vector_fe_weak_divergence_kernel.h"

namespace hephaestus
{

VectorFEWeakDivergenceKernel::VectorFEWeakDivergenceKernel(
    const hephaestus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
VectorFEWeakDivergenceKernel::Init(hephaestus::GridFunctions & gridfunctions,
                                   const hephaestus::FESpaces & fespaces,
                                   hephaestus::BCMap & bc_map,
                                   Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
VectorFEWeakDivergenceKernel::Apply(mfem::ParMixedBilinearForm * mblf)
{
  mblf->AddDomainIntegrator(new mfem::VectorFEWeakDivergenceIntegrator(*_coef));
};

} // namespace hephaestus
