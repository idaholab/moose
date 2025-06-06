#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSubMesh.h"

/**
 * Constructs and stores an mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
 */
class MFEMDomainSubMesh : public MFEMSubMesh
{
public:
  static InputParameters validParams();
  MFEMDomainSubMesh(const InputParameters & parameters);
  const mfem::Array<int> & getSubdomains() { return _subdomain_attributes; }

protected:
  virtual void buildSubMesh() override;

  const std::vector<SubdomainName> & _subdomain_names;
  mfem::Array<int> _subdomain_attributes;
};

#endif
