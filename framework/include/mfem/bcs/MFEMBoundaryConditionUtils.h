#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"
#include "MFEMEssentialBC.h"
#include "MFEMContainers.h"

namespace platypus
{

class BCMap : public platypus::NamedFieldsMap<MFEMBoundaryCondition>
{
public:
  mfem::Array<int> GetEssentialBdrMarkers(const std::string & name_, mfem::Mesh * mesh_);

  void ApplyEssentialBCs(const std::string & name_,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_, mfem::BilinearForm & blf, mfem::Mesh * mesh_);

  void ApplyIntegratedBCs(const std::string & name_, mfem::LinearForm & lf, mfem::Mesh * mesh_);
};

} // namespace platypus

#endif
