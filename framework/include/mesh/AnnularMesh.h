//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

/**
 * Mesh generated from parameters
 */
class AnnularMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  AnnularMesh(const InputParameters & parameters);
  AnnularMesh(const AnnularMesh & /* other_mesh */) = default;

  // No copy
  AnnularMesh & operator=(const AnnularMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;
  virtual void prepared(bool state) override;

protected:
  /// Number of elements in radial direction
  const unsigned _nr;

  /// Number of elements in angular direction
  const unsigned _nt;

  /// Minimum radius
  const Real _rmin;

  /// Maximum radius
  const Real _rmax;

  /// Minimum angle in degrees
  const Real _dmin;

  /// Maximum angle in degrees
  const Real _dmax;

  /// Bool to check if radians are given in the input file
  const bool _radians;

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

  /// Boolean to indicate that dimensions may have changed
  bool _dims_may_have_changed;
};
