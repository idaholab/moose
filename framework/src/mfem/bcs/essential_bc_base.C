#include "essential_bc_base.h"

namespace hephaestus
{

EssentialBC::EssentialBC(const std::string & name_, mfem::Array<int> bdr_attributes_)
  : BoundaryCondition(name_, bdr_attributes_)
{
}

} // namespace hephaestus
