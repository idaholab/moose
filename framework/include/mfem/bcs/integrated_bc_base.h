#pragma once
#include "boundary_condition_base.h"

namespace platypus
{

class IntegratedBC : public BoundaryCondition
{
public:
  IntegratedBC(const std::string & name_, mfem::Array<int> bdr_attributes_);
  IntegratedBC(const std::string & name_,
               mfem::Array<int> bdr_attributes_,
               std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_);

  std::unique_ptr<mfem::LinearFormIntegrator> _lfi_re;

  void ApplyBC(mfem::LinearForm & b) override;
};

} // namespace platypus
