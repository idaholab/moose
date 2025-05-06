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
class MFEMSubMesh : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMSubMesh(const InputParameters & parameters);
  mfem::Array<int> & getSubdomains() { return _subdomain_attributes; }

  /// Returns a shared pointer to the constructed fespace.
  inline std::shared_ptr<mfem::ParSubMesh> getSubMesh()
  {
    if (!_submesh)
      buildSubMesh();
    return _submesh;
  }

protected:
  std::vector<SubdomainName> _subdomain_names;
  mfem::Array<int> _subdomain_attributes;

private:
  /// Constructs the submesh.
  void buildSubMesh();
  /// Stores the constructed submesh.
  mutable std::shared_ptr<mfem::ParSubMesh> _submesh{nullptr};
};

#endif
