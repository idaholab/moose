//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Base class for construction of a mfem::ParSubMesh object. Access using the
 * getSubMesh() accessor.
 */
class MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMBlockRestrictable(const InputParameters & parameters, const mfem::ParMesh & mfem_mesh);

  mfem::Array<int> subdomainsToAttributes(const std::vector<SubdomainName> & subdomain_names);
  std::vector<std::string> subdomainsToStrings(const std::vector<SubdomainName> & subdomain_names);

  /// Returns a shared pointer to the constructed fespace.
  bool isSubdomainRestricted() { return _subdomain_names.size(); }

  mfem::Array<int> & getSubdomains() { return _subdomain_markers; }
  const mfem::ParMesh & getMesh() { return _mfem_mesh; }

protected:
  /// Stores the names of the subdomains.
  const mfem::ParMesh & _mfem_mesh;
  std::vector<SubdomainName> _subdomain_names;
  mfem::Array<int> _subdomain_attributes;
  mfem::Array<int> _subdomain_markers;
};

#endif
