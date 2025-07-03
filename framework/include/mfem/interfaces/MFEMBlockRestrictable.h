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
#include "GeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Base class for construction of an object that is restricted to a subset
 * of subdomains of the problem mesh.
 */
class MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMBlockRestrictable(const InputParameters & parameters, const mfem::ParMesh & mfem_mesh);

  mfem::Array<int> subdomainsToAttributes(const std::vector<SubdomainName> & subdomain_names);
  std::vector<std::string> subdomainsToStrings(const std::vector<SubdomainName> & subdomain_names);

  /// Returns a bool indicating if the object is restricted to a subset of subdomains.
  bool isSubdomainRestricted() { return _subdomain_names.size(); }

  const mfem::Array<int> & getSubdomainAttributes() { return _subdomain_attributes; }
  mfem::Array<int> & getSubdomainMarkers() { return _subdomain_markers; }
  const mfem::ParMesh & getMesh() const { return _mfem_mesh; }

protected:
  /// Stores the names of the subdomains.
  const mfem::ParMesh & _mfem_mesh;
  /// Stores the names of the subdomains.
  std::vector<SubdomainName> _subdomain_names;
  /// Array storing subdomain attribute IDs for this object.
  mfem::Array<int> _subdomain_attributes;
  /// Boolean array indicating which subdomains are active in this object.
  mfem::Array<int> _subdomain_markers;
};

#endif
