#pragma once
#include "essential_bc_base.hpp"

namespace hephaestus
{

class VectorDirichletBC : public EssentialBC
{

protected:
  enum APPLY_TYPE
  {
    STANDARD,
    TANGENTIAL,
    NORMAL
  };

public:
  VectorDirichletBC(const std::string & name_, mfem::Array<int> bdr_attributes_);
  VectorDirichletBC(const std::string & name_,
                    mfem::Array<int> bdr_attributes_,
                    mfem::VectorCoefficient * vec_coeff_,
                    mfem::VectorCoefficient * vec_coeff_im_ = nullptr,
                    APPLY_TYPE boundary_apply_type = TANGENTIAL);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;

  void ApplyBC(mfem::ParComplexGridFunction & gridfunc, mfem::Mesh * mesh_) override;

  mfem::VectorCoefficient * _vec_coeff{nullptr};
  mfem::VectorCoefficient * _vec_coeff_im{nullptr};
  APPLY_TYPE _boundary_apply_type;
};

} // namespace hephaestus
