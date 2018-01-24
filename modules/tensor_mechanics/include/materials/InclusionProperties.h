//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INCLUSIONPROPERTIES_H
#define INCLUSIONPROPERTIES_H

#include "Material.h"
#include "RankTwoTensor.h"

// Forward Declarations
class InclusionProperties;

template <>
InputParameters validParams<InclusionProperties>();

/**
 * This material calculates the stresses, strains, and elastic energies for an
 * ellipsoidal inclusion in a 2D, plane strain configuration with in-plane
 * dilatational eigenstrains only. Both inside and outside the inclusion are
 * calculated. References: X. Jin et al., J. Elast., v. 114, 1-18 (2014) and
 * X. Jin et al., J. Appl. Mech., v. 78, 031009 (2011).
 */
class InclusionProperties : public Material
{
public:
  InclusionProperties(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void precomputeInteriorProperties();

private:
  /// Semimajor axes of the ellipsoidal inclusion
  const Real _a;
  const Real _b;

  /// Elastic constants (isotropic)
  const Real _lambda;
  const Real _mu;

  /// Misfit strains
  std::vector<Real> _misfit;

  /// Poisson's ratio
  Real _nu;
  /// Kolosov's first constant
  Real _kappa;

  /**
   * Interior stress and strain values are constant so they only need to be
   * calculated once
   */
  RankTwoTensor _stress_int;
  RankTwoTensor _total_strain_int;
  RankTwoTensor _elastic_strain_int;
  Real _elastic_energy_int;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _strain;
  MaterialProperty<Real> & _elastic_energy;
};

#endif // INCLUSIONPROPERTIES_H
