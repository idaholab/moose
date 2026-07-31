//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * Computes power-law strain energy rate density using fifth-order Gaussian quadrature.
 */
class PowerLawNumericalStrainEnergyRateDensity : public Material
{
public:
  static InputParameters validParams();

  PowerLawNumericalStrainEnergyRateDensity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<Real> & _strain_energy_rate_density;
  const Real _n_exponent;
};
