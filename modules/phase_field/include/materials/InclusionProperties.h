/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef INCLUSIONPROPERTIES_H
#define INCLUSIONPROPERTIES_H

#include "Material.h"
#include "RankTwoTensor.h"

//Forward Declarations
class InclusionProperties;

template<>
InputParameters validParams<InclusionProperties>();

/**
 * This material calculates the stresses, strains, and elastic energies for an
 * ellipsoidal inclusion in a 2D, plane strain configuration with in-plane
 * dilatational eigenstrains only. Both inside and outside the inclusion are
 * calculated. Reference: X. Jin, J. Elast., v. 114, 1-18 (2014).
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
  RankTwoTensor _strain_int;
  Real _elastic_energy_int;

  /// Material property names
  MaterialPropertyName _stress_name;
  MaterialPropertyName _strain_name;
  MaterialPropertyName _energy_name;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _strain;
  MaterialProperty<Real> & _elastic_energy;
};

#endif //INCLUSIONPROPERTIES_H
