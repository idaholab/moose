#ifdef MFEM_ENABLED

#include "MFEMBoundaryConditionUtils.h"

namespace Moose::MFEM
{

mfem::Array<int>
BCMap::GetEssentialBdrMarkers(const std::string & test_var_name, mfem::Mesh & mesh)
{
  mfem::Array<int> global_ess_markers(mesh.bdr_attributes.Max());
  global_ess_markers = 0;
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = 0;

  for (auto const & [bc_name, bc] : *this)
  {
    if (bc->getTestVariableName() == test_var_name)
    {
      auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc);
      if (essential_bc != nullptr)
      {
        ess_bdrs = essential_bc->GetMarkers(mesh);
        for (auto it = 0; it != mesh.bdr_attributes.Max(); ++it)
          global_ess_markers[it] = std::max(global_ess_markers[it], ess_bdrs[it]);
      }
    }
  }
  return global_ess_markers;
}

void
BCMap::ApplyEssentialBCs(const std::string & test_var_name,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh & mesh)
{
  for (auto const & [bc_name, bc] : *this)
    if (bc->getTestVariableName() == test_var_name)
    {
      auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc);
      if (essential_bc != nullptr)
        essential_bc->ApplyBC(gridfunc, mesh);
    }
  mfem::Array<int> ess_bdr = GetEssentialBdrMarkers(test_var_name, mesh);
  gridfunc.FESpace()->GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
}

void
BCMap::ApplyIntegratedBCs(const std::string & test_var_name,
                          mfem::LinearForm & lf,
                          mfem::Mesh & mesh)
{
  for (auto const & [bc_name, bc] : *this)
  {
    if (bc->getTestVariableName() != test_var_name)
      continue;

    auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc);
    if (!integrated_bc)
      continue;

    integrated_bc->GetMarkers(mesh);
    mfem::LinearFormIntegrator * lfi = integrated_bc->createLinearFormIntegrator();
    if (!lfi)
      continue;

    lf.AddBoundaryIntegrator(lfi, integrated_bc->_bdr_markers);
  }
}

void
BCMap::ApplyIntegratedBCs(const std::string & test_var_name,
                          mfem::BilinearForm & blf,
                          mfem::Mesh & mesh)
{
  for (auto const & [bc_name, bc] : *this)
  {
    if (bc->getTestVariableName() != test_var_name)
      continue;

    auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc);
    if (!integrated_bc)
      continue;

    integrated_bc->GetMarkers(mesh);
    mfem::BilinearFormIntegrator * blfi = integrated_bc->createBilinearFormIntegrator();
    if (!blfi)
      continue;

    blf.AddBoundaryIntegrator(blfi, integrated_bc->_bdr_markers);
  }
}

} // namespace Moose::MFEM

#endif
