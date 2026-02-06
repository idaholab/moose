//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubdomainsGeneratorBase.h"

/**
 * This class will add subdomains to the entire mesh based on unique normals.
 * Note: The user will have to turn the tolerance on the normal angle to be able to
 * "paint" curved subdomains. Tight tolerances will create one subdomain per element
 */
class SurfaceSubdomainsFromAllNormalsGenerator : public SubdomainsGeneratorBase
{
public:
  static InputParameters validParams();

  SurfaceSubdomainsFromAllNormalsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Map from subdomain IDs to the normals of the corresponding boundaries
  std::map<SubdomainID, RealVectorValue> _subdomain_to_normal_map;
  /// Whether to only use the flood algorithm to group elements, no checks in "previous normals"
  const bool _flood_only;
};
