#pragma once
#include "essential_bcs.h"
#include "integrated_bcs.h"
#include "named_fields_map.h"
#include "robin_bcs.h"

namespace hephaestus
{

class BCMap : public hephaestus::NamedFieldsMap<hephaestus::BoundaryCondition>
{
public:
  mfem::Array<int> GetEssentialBdrMarkers(const std::string & name_, mfem::Mesh * mesh_);

  void ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh * mesh_);

  void ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::ParComplexGridFunction & gridfunc,
                         mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_, mfem::LinearForm & lf, mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_,
                          mfem::ParComplexLinearForm & clf,
                          mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_,
                          mfem::ParSesquilinearForm & clf,
                          mfem::Mesh * mesh_);
};

} // namespace hephaestus
