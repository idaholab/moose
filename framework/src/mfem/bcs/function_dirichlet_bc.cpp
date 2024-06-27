#include "function_dirichlet_bc.hpp"

namespace hephaestus
{

ScalarDirichletBC::ScalarDirichletBC(const std::string & name_, mfem::Array<int> bdr_attributes_)
  : EssentialBC(name_, bdr_attributes_)
{
}

ScalarDirichletBC::ScalarDirichletBC(const std::string & name_,
                                     mfem::Array<int> bdr_attributes_,
                                     mfem::Coefficient * coeff_,
                                     mfem::Coefficient * coeff_im_)
  : EssentialBC(name_, bdr_attributes_), _coeff{coeff_}, _coeff_im{coeff_im_}
{
}

void
ScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(*(_coeff), ess_bdrs);
}

} // namespace hephaestus
