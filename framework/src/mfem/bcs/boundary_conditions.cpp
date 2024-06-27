#include "boundary_conditions.hpp"

namespace hephaestus
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
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::EssentialBC>(bc_);
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
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::EssentialBC>(bc_);
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
BCMap::ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::ParComplexGridFunction & gridfunc,
                         mfem::Mesh * mesh_)
{
  for (auto const & [name, bc_] : *this)
  {
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::EssentialBC>(bc_);
      if (bc != nullptr)
      {
        bc->ApplyBC(gridfunc, mesh_);
      }
    }
  }
  mfem::Array<int> ess_bdr = GetEssentialBdrMarkers(name_, mesh_);
  gridfunc.FESpace()->GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
};

void
BCMap::ApplyIntegratedBCs(const std::string & name_, mfem::LinearForm & lf, mfem::Mesh * mesh_)
{

  for (auto const & [name, bc_] : *this)
  {
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::IntegratedBC>(bc_);
      if (bc != nullptr)
      {
        bc->GetMarkers(*mesh_);
        bc->ApplyBC(lf);
      }
    }
  }
};

void
BCMap::ApplyIntegratedBCs(const std::string & name_,
                          mfem::ParComplexLinearForm & clf,
                          mfem::Mesh * mesh_)
{

  for (auto const & [name, bc_] : *this)
  {
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::IntegratedBC>(bc_);
      if (bc != nullptr)
      {
        bc->GetMarkers(*mesh_);
        bc->ApplyBC(clf);
      }
    }
  }
};

void
BCMap::ApplyIntegratedBCs(const std::string & name_,
                          mfem::ParSesquilinearForm & slf,
                          mfem::Mesh * mesh_)
{
  for (auto const & [name, bc_] : *this)
  {
    if (bc_->_name == name_)
    {
      auto bc = std::dynamic_pointer_cast<hephaestus::RobinBC>(bc_);
      if (bc != nullptr)
      {
        bc->GetMarkers(*mesh_);
        bc->ApplyBC(slf);
      }
    }
  }
};

} // namespace hephaestus
