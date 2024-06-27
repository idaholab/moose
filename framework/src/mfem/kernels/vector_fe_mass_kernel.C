#include "vector_fe_mass_kernel.h"

namespace platypus
{

VectorFEMassKernel::VectorFEMassKernel(const platypus::InputParameters & params)
  : Kernel(params), _coef_name(params.GetParam<std::string>("CoefficientName"))
{
}

void
VectorFEMassKernel::Init(platypus::GridFunctions & gridfunctions,
                         const platypus::FESpaces & fespaces,
                         platypus::BCMap & bc_map,
                         Coefficients & coefficients)
{
  _coef = coefficients._scalars.Get(_coef_name);
}

void
VectorFEMassKernel::Apply(mfem::ParBilinearForm * blf)
{
  blf->AddDomainIntegrator(new mfem::VectorFEMassIntegrator(*_coef));
};

} // namespace platypus
