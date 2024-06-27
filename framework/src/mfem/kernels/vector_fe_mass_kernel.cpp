#include "vector_fe_mass_kernel.hpp"

namespace hephaestus
{

VectorFEMassKernel::VectorFEMassKernel(const hephaestus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
VectorFEMassKernel::Init(hephaestus::GridFunctions & gridfunctions,
                         const hephaestus::FESpaces & fespaces,
                         hephaestus::BCMap & bc_map,
                         hephaestus::Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
VectorFEMassKernel::Apply(mfem::ParBilinearForm * blf)
{
  blf->AddDomainIntegrator(new mfem::VectorFEMassIntegrator(*_coef));
};

} // namespace hephaestus
