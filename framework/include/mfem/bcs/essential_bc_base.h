#pragma once
#include "boundary_condition_base.h"

namespace platypus
{

class EssentialBC : public BoundaryCondition
{
public:
  EssentialBC(const std::string & name_, mfem::Array<int> bdr_attributes_);

  virtual void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) {}
};

} // namespace platypus
