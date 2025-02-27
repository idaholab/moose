#ifdef MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include "MFEMGeneralUserObject.h"
#include "PropertyManager.h"

class MFEMMaterial : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();
  static std::vector<std::string> subdomainsToStrings(std::vector<SubdomainName> blocks);
  static libMesh::Point pointFromMFEMVector(const mfem::Vector & vec);

  MFEMMaterial(const InputParameters & parameters);
  virtual ~MFEMMaterial();

  const std::vector<SubdomainName> & getBlocks() const { return _block_ids; }

protected:
  std::vector<SubdomainName> _block_ids;
  MooseMFEM::PropertyManager & _properties;
};

#endif
