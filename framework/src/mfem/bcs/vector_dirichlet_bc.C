#include "vector_dirichlet_bc.h"

namespace platypus
{

VectorDirichletBC::VectorDirichletBC(const std::string & name_, mfem::Array<int> bdr_attributes_)
  : EssentialBC(name_, bdr_attributes_)
{
}

VectorDirichletBC::VectorDirichletBC(const std::string & name_,
                                     mfem::Array<int> bdr_attributes_,
                                     mfem::VectorCoefficient * vec_coeff_,
                                     APPLY_TYPE boundary_apply_type_)
  : EssentialBC(name_, bdr_attributes_),
    _vec_coeff{vec_coeff_},
    _boundary_apply_type{boundary_apply_type_}
{
}

void
VectorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  if (_vec_coeff == nullptr)
  {
    MFEM_ABORT("Boundary condition does not store valid coefficients to specify the "
               "components of the vector at the Dirichlet boundary.");
  }

  switch (_boundary_apply_type)
  {
    case STANDARD:
      gridfunc.ProjectBdrCoefficient(*(_vec_coeff), ess_bdrs);
      break;
    case NORMAL:
      gridfunc.ProjectBdrCoefficientNormal(*(_vec_coeff), ess_bdrs);
      break;
    case TANGENTIAL:
      gridfunc.ProjectBdrCoefficientTangent(*(_vec_coeff), ess_bdrs);
  }
}

} // namespace platypus
