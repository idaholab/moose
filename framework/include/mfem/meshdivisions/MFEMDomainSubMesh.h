#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMGeneralUserObject.h"

/**
 * Constructs and stores an mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
 */
class MFEMDomainSubMesh : public MFEMSubMeshBase
{
public:
  static InputParameters validParams();
  MFEMDomainSubMesh(const InputParameters & parameters);
  mfem::Array<int> & getSubdomains() { return _subdomain_attributes; }

protected:
  virtual void buildSubMesh() override;

  std::vector<SubdomainName> _subdomain_names;
  mfem::Array<int> _subdomain_attributes;
};

#endif
