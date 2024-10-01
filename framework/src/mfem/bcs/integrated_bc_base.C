#include "integrated_bc_base.h"

namespace platypus
{

IntegratedBC::IntegratedBC(const std::string & name_, mfem::Array<int> bdr_attributes_)
  : BoundaryCondition(name_, bdr_attributes_)
{
}

IntegratedBC::IntegratedBC(const std::string & name_,
                           mfem::Array<int> bdr_attributes_,
                           std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_)
  : BoundaryCondition(name_, bdr_attributes_), _lfi_re{std::move(lfi_re_)}
{
}

void
IntegratedBC::ApplyBC(mfem::LinearForm & b)
{
  // NB: release ownership to prevent double-free. LinearForm assumes ownership.
  b.AddBoundaryIntegrator(_lfi_re.release(), _markers);
}

} // namespace platypus
