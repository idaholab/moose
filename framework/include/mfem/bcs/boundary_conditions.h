#pragma once
#include "essential_bcs.h"
#include "integrated_bcs.h"
#include "MFEMContainers.h"
#include "robin_bcs.h"

namespace platypus
{

class BCMap : public platypus::NamedFieldsMap<platypus::BoundaryCondition>
{
public:
  mfem::Array<int> GetEssentialBdrMarkers(const std::string & name_, mfem::Mesh * mesh_);

  void ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_, mfem::LinearForm & lf, mfem::Mesh * mesh_);
};

} // namespace platypus
