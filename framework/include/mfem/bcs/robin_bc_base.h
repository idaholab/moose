#pragma once
#include "integrated_bc_base.h"

namespace platypus
{

class RobinBC : public IntegratedBC
{
public:
  RobinBC(const std::string & name_,
          mfem::Array<int> bdr_attributes_,
          std::unique_ptr<mfem::BilinearFormIntegrator> blfi_re_,
          std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_);

  std::unique_ptr<mfem::BilinearFormIntegrator> _blfi_re{nullptr};

  virtual void ApplyBC(mfem::ParBilinearForm & a);
};

} // namespace platypus
