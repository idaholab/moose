#include "robin_bc_base.h"

namespace platypus
{

RobinBC::RobinBC(const std::string & name_,
                 mfem::Array<int> bdr_attributes_,
                 std::unique_ptr<mfem::BilinearFormIntegrator> blfi_re_,
                 std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_)
  : IntegratedBC(name_, bdr_attributes_, std::move(lfi_re_)), _blfi_re{std::move(blfi_re_)}
{
}

void
RobinBC::ApplyBC(mfem::ParBilinearForm & a)
{
  // NB: ParBilinearForm assumes ownership of pointers. Release to
  // prevent double-free!
  a.AddBoundaryIntegrator(_blfi_re.release(), _markers);
}
} // namespace platypus
