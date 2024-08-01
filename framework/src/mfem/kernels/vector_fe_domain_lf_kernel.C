#include "vector_fe_domain_lf_kernel.h"

namespace platypus
{

VectorFEDomainLFKernel::VectorFEDomainLFKernel(const platypus::InputParameters & params)
  : Kernel(params), _vec_coef_name(params.GetParam<std::string>("VectorCoefficientName"))
{
}

void
VectorFEDomainLFKernel::Init(platypus::GridFunctions & gridfunctions,
                             const platypus::FESpaces & fespaces,
                             platypus::BCMap & bc_map,
                             platypus::Coefficients & coefficients)
{
  _vec_coef = coefficients._vectors.Get(_vec_coef_name);
}

void
VectorFEDomainLFKernel::Apply(mfem::ParLinearForm * lf)
{
  lf->AddDomainIntegrator(new mfem::VectorFEDomainLFIntegrator(*_vec_coef));
  lf->Assemble();
}

} // namespace platypus
