//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ANNULARMESHGENERATOR_H
#define ANNULARMESHGENERATOR_H

#include "MeshGenerator.h"

// Forward declarations
class AnnularMeshGenerator;

template <>
InputParameters validParams<AnnularMeshGenerator>();

/**
 * Generates an annular mesh given all the parameters
 */
class AnnularMeshGenerator : public MeshGenerator
{
public:
  AnnularMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Number of elements in radial direction
  const unsigned _nr;

  /// Number of elements in angular direction
  const unsigned _nt;

  /// Minimum radius
  const Real _rmin;

  /// Maximum radius
  const Real _rmax;

  /// Minimum angle
  const Real _tmin;

  /// Maximum angle
  const Real _tmax;

  /// Bias on radial meshing
  const Real _growth_r;

  /// rmax = rmin + len + len*g + len*g^2 + len*g^3 + ... + len*g^(nr-1) = rmin + len*(1 - g^nr)/(1 - g)
  const Real _len;

  /// Whether a full annulus (as opposed to a sector) will needs to generate
  const bool _full_annulus;

  /// Subdomain ID of created quad elements
  const SubdomainID _quad_subdomain_id;

  /// Subdomain ID of created tri elements (that only exist if rmin=0)
  const SubdomainID _tri_subdomain_id;
};

#endif // ANNULARMESHGENERATOR_H
