#include "integrated_bc_base.hpp"

namespace hephaestus
{

IntegratedBC::IntegratedBC(const std::string & name_, mfem::Array<int> bdr_attributes_)
  : BoundaryCondition(name_, bdr_attributes_)
{
}

IntegratedBC::IntegratedBC(const std::string & name_,
                           mfem::Array<int> bdr_attributes_,
                           std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_,
                           std::unique_ptr<mfem::LinearFormIntegrator> lfi_im_)
  : BoundaryCondition(name_, bdr_attributes_),
    _lfi_re{std::move(lfi_re_)},
    _lfi_im{std::move(lfi_im_)}
{
}

void
IntegratedBC::ApplyBC(mfem::LinearForm & b)
{
  // NB: release ownership to prevent double-free. LinearForm assumes ownership.
  b.AddBoundaryIntegrator(_lfi_re.release(), _markers);
}

void
IntegratedBC::ApplyBC(mfem::ComplexLinearForm & b)
{
  b.AddBoundaryIntegrator(_lfi_re.release(), _lfi_im.release(), _markers);
}

void
IntegratedBC::ApplyBC(mfem::ParComplexLinearForm & b)
{
  b.AddBoundaryIntegrator(_lfi_re.release(), _lfi_im.release(), _markers);
}

} // namespace hephaestus
