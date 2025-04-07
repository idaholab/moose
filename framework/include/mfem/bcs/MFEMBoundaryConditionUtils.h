#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"
#include "MFEMEssentialBC.h"
#include "MFEMContainers.h"

namespace Moose::MFEM
{

class BCMap : public Moose::MFEM::NamedFieldsMap<MFEMBoundaryCondition>
{
public:
  void ApplyEssentialBCs(const std::string & test_var_name,
                         mfem::Array<int> & ess_tdof_list,
                         mfem::GridFunction & gridfunc,
                         mfem::Mesh & mesh);

  void ApplyIntegratedBCs(const std::string & test_var_name,
                          mfem::BilinearForm & blf,
                          mfem::Mesh & mesh);

  void
  ApplyIntegratedBCs(const std::string & test_var_name, mfem::LinearForm & lf, mfem::Mesh & mesh);

private:
  mfem::Array<int> GetEssentialBdrMarkers(const std::string & test_var_name, mfem::Mesh & mesh);
};

} // namespace Moose::MFEM

#endif
