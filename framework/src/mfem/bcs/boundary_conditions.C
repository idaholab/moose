#ifdef MFEM_ENABLED

#include "boundary_conditions.h"

namespace platypus
{

mfem::Array<int>
BCMap::GetEssentialBdrMarkers(const std::string & name_, mfem::Mesh * mesh_)
{
  mfem::Array<int> global_ess_markers(mesh_->bdr_attributes.Max());
  global_ess_markers = 0;
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = 0;

  for (auto const & [name, bc_] : *this)
  {
    if (bc_->getTestVariableName() == name_)
    {
      auto bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc_);
      if (bc != nullptr)
      {
        ess_bdrs = bc->GetMarkers(*mesh_);
        for (auto it = 0; it != mesh_->bdr_attributes.Max(); ++it)
        {
          global_ess_markers[it] = std::max(global_ess_markers[it], ess_bdrs[it]);
        }
      }
    }
  }
  return global_ess_markers;
}

void
BCMap::ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh * mesh_)
{
  for (auto const & [name, bc_] : *this)
  {
    if (bc_->getTestVariableName() == name_)
    {
      auto bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc_);
      if (bc != nullptr)
      {
        bc->ApplyBC(gridfunc, mesh_);
      }
    }
  }
  mfem::Array<int> ess_bdr = GetEssentialBdrMarkers(name_, mesh_);
  gridfunc.FESpace()->GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
}

void
BCMap::ApplyIntegratedBCs(const std::string & name_, mfem::LinearForm & lf, mfem::Mesh * mesh_)
{
  for (auto const & [name, bc_] : *this)
  {
    if (bc_->getTestVariableName() != name_)
    {
      continue;
    }

    auto bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc_);
    if (!bc)
    {
      continue;
    }

    bc->GetMarkers(*mesh_);
    mfem::LinearFormIntegrator * lfi = bc->createLinearFormIntegrator();
    if (!lfi)
    {
      continue;
    }

    lf.AddBoundaryIntegrator(lfi, bc->_bdr_markers);
  }
};

void
BCMap::ApplyIntegratedBCs(const std::string & name_, mfem::BilinearForm & blf, mfem::Mesh * mesh_)
{
  for (auto const & [name, bc_] : *this)
  {
    if (bc_->getTestVariableName() != name_)
    {
      continue;
    }

    auto bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc_);
    if (!bc)
    {
      continue;
    }

    bc->GetMarkers(*mesh_);
    mfem::BilinearFormIntegrator * blfi = bc->createBilinearFormIntegrator();
    if (!blfi)
    {
      continue;
    }

    blf.AddBoundaryIntegrator(blfi, bc->_bdr_markers);
  }
};

} // namespace platypus

#endif
