#pragma once
#include "integrated_bc_base.hpp"

namespace hephaestus
{

class RobinBC : public IntegratedBC
{
public:
  RobinBC(const std::string & name_,
          mfem::Array<int> bdr_attributes_,
          std::unique_ptr<mfem::BilinearFormIntegrator> blfi_re_,
          std::unique_ptr<mfem::LinearFormIntegrator> lfi_re_,
          std::unique_ptr<mfem::BilinearFormIntegrator> blfi_im_ = nullptr,
          std::unique_ptr<mfem::LinearFormIntegrator> lfi_im_ = nullptr);

  std::unique_ptr<mfem::BilinearFormIntegrator> _blfi_re{nullptr};
  std::unique_ptr<mfem::BilinearFormIntegrator> _blfi_im{nullptr};

  virtual void ApplyBC(mfem::ParBilinearForm & a);
  virtual void ApplyBC(mfem::ParSesquilinearForm & a);
};

} // namespace hephaestus
