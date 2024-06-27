#include "robin_bc_base.hpp"

namespace hephaestus
{

RobinBC::RobinBC(const std::string & name_,
                 mfem::Array<int> bdr_attributes_,
                 std::unique_ptr<mfem::BilinearFormIntegrator> blfi_re_,
                 std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_,
                 std::unique_ptr<mfem::BilinearFormIntegrator> blfi_im_,
                 std::unique_ptr<mfem::LinearFormIntegrator> lfi_im_)
  : IntegratedBC(name_, bdr_attributes_, std::move(lfi_re_), std::move(lfi_im_)),
    _blfi_re{std::move(blfi_re_)},
    _blfi_im{std::move(blfi_im_)}
{
}

void
RobinBC::ApplyBC(mfem::ParBilinearForm & a)
{
  // NB: ParBilinearForm assumes ownership of pointers. Release to
  // prevent double-free!
  a.AddBoundaryIntegrator(_blfi_re.release(), _markers);
}

void
RobinBC::ApplyBC(mfem::ParSesquilinearForm & a)
{
  a.AddBoundaryIntegrator(_blfi_re.release(), _blfi_im.release(), _markers);
}

} // namespace hephaestus
