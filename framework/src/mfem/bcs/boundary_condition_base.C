#include "boundary_condition_base.h"

#include <utility>

namespace platypus
{

BoundaryCondition::BoundaryCondition(std::string name_, mfem::Array<int> bdr_attributes_)
  : _name(std::move(name_)), _bdr_attributes(std::move(bdr_attributes_))
{
}

mfem::Array<int>
BoundaryCondition::GetMarkers(mfem::Mesh & mesh)
{
  mfem::common::AttrToMarker(mesh.bdr_attributes.Max(), _bdr_attributes, _markers);
  return _markers;
}

} // namespace platypus
