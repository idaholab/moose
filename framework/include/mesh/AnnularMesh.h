/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ANNULARMESH_H
#define ANNULARMESH_H

#include "MooseMesh.h"

class AnnularMesh;

template <>
InputParameters validParams<AnnularMesh>();

/**
 * Mesh generated from parameters
 */
class AnnularMesh : public MooseMesh
{
public:
  AnnularMesh(const InputParameters & parameters);
  AnnularMesh(const AnnularMesh & other_mesh) = default;

  // No copy
  AnnularMesh & operator=(const AnnularMesh & other_mesh) = delete;

  virtual MooseMesh & clone() const override;
  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;

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

#endif /* ANNULARMESH_H */
